#include "data/python/wrapper.h"
#include "data/signal.h"
#include "data/nodes/data_node.h"
#include "data/debuglog.h"

#include "QWidget"

#include <chrono>

#include "viewer.h"

using namespace MindTree;

std::thread WorkerThread::_updateThread;
std::mutex WorkerThread::_updateMutex;
std::condition_variable WorkerThread::_needToUpdateCondition;
std::atomic<bool> WorkerThread::_running(false);

std::vector<UpdateInfo> WorkerThread::_updateQueue;
std::atomic<uint32_t> WorkerThread::_updateQueueSize{0};

struct MindTree::UpdateInfo {
	UpdateInfo(Viewer* viewer, DNode *node) : _viewer(viewer), _node(node) {}
    Viewer* _viewer;
    DNode* _node;
};

bool WorkerThread::needToUpdate()
{
    // std::lock_guard<std::mutex> lock(_updateMutex);
    // return !_updateQueue.empty();
	return _updateQueueSize > 0;
}

void WorkerThread::notifyUpdate()
{
    {
        // std::lock_guard<std::mutex> lock(_updateMutex);
        ++_updateQueueSize;
        // _updateQueue.push_back(info);
    }
    if(!_running) start();
}

void WorkerThread::update(UpdateInfo info)
{
    {
        std::lock_guard<std::mutex> lock(_updateMutex);
        _updateQueue.push_back(info);
    }

    _needToUpdateCondition.notify_all();
}

void WorkerThread::removeViewer(Viewer *viewer)
{
    std::cout << "remove viewer ..." << std::endl;
    {
        // std::lock_guard<std::mutex> lock(_updateMutex);
        // auto it = std::find_if(begin(_updateQueue),
        //                        end(_updateQueue),
        //                        [viewer] (std::weak_ptr<UpdateInfo> i) {
        //                        return i.lock()->_viewer == viewer;
        //                        });
        // if(it != end(_updateQueue))
        //     _updateQueue.erase(it);
	    if (_updateQueueSize > 0) --_updateQueueSize;
    }

    std::cout << "removed viewer" << std::endl;

    if(!needToUpdate()) stop();
}

void WorkerThread::start()
{
    std::cout << "starting update Thread" << std::endl;
    if(_updateThread.joinable()) _updateThread.join();

    auto updateFunc = []{
        WorkerThread::_running = true;
        while(WorkerThread::needToUpdate()) {
            std::unique_lock<std::mutex> lock(_updateMutex);
            _needToUpdateCondition.wait(lock);

            // for(auto &info : _updateQueue) {
            while (!_updateQueue.empty()) {
	            auto info = _updateQueue.back();
	            _updateQueue.pop_back();
                // if(!info.expired()) {
                //     auto ptr = info.lock();
				if(info._node) DataCache::invalidate(info._node);
				info._viewer->cacheAndUpdate();
                // }
            }
        }
        WorkerThread::_running = false;
    };

    _updateThread = std::thread(updateFunc);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2ms);
}

void WorkerThread::stop()
{
    {
        std::lock_guard<std::mutex> lock(_updateMutex);
        _updateQueue.clear();
    }
	_updateQueueSize = 0;
	_needToUpdateCondition.notify_all();

    if (_updateThread.joinable()) _updateThread.join();
	std::cout << "Stoped Update Thread" << std::endl;
}

Viewer::Viewer(DoutSocket *start)
    : widget(0),
    start(start),
    _signalLiveTime(new Signal::LiveTimeTracker(this))
{
    auto cbhandler = Signal::getHandler<DinSocket*>()
        .connect("createLink",
                std::bind(static_cast<void (Viewer::*)(DinSocket*)>(&Viewer::update_viewer),
                          this,
                          std::placeholders::_1));

    auto cbhandler2 = Signal::getHandler<DinSocket*>()
        .connect("socketChanged",
                std::bind(static_cast<void (Viewer::*)(DinSocket*)>(&Viewer::update_viewer),
                          this,
                          std::placeholders::_1));

    auto cbhandler3 = Signal::getHandler<DNode*>()
        .connect("nodeChanged",
                std::bind(static_cast<void (Viewer::*)(DNode*)>(&Viewer::update_viewer),
                          this,
                          std::placeholders::_1));

    cbhandlers.push_back(cbhandler);
    cbhandlers.push_back(cbhandler2);
    cbhandlers.push_back(cbhandler3);
}

Viewer::~Viewer()
{
    WorkerThread::removeViewer(this);
}

void Viewer::initBase()
{
    init();
    WorkerThread::notifyUpdate();
    update_viewer(static_cast<DinSocket*>(nullptr));
}

void Viewer::update_viewer(DNode *node)
{
    //check whether start and socket are connected
    bool connected = false;
    if (!node || node == start->getNode()) {
        connected = true;
    }
    else {
        ConstNodeList nodes = start->getNode()->getAllInNodesConst();
        for(const DNode *n : nodes) {
            if (n == node) {
                connected = true;
                break;
            }
        }
    }

    if(connected) {
	    WorkerThread::update(UpdateInfo(this, node));
    }
}

void Viewer::update_viewer(DinSocket *socket)
{
    DNode *node = nullptr;
    if(socket) node = socket->getNode();
    //check whether start and socket are connected
    bool connected = false;
    if (!socket || socket->getNode() == start->getNode()
        || socket->getNode() == _settingsNode.get()) {
        connected = true;
    }
    else {
        ConstNodeList nodes = start->getNode()->getAllInNodesConst();
        for(const DNode *node : nodes) {
            for (const DinSocket *in : node->getInSockets()) {
                if (in == socket) {
                    connected = true;
                    break;
                }
            }
            if(connected) break;
        }
        if(_settingsNode) {
            ConstNodeList settingsNodes = _settingsNode->getAllInNodesConst();
            for(const DNode *node : settingsNodes) {
                for (const DinSocket *in : node->getInSockets()) {
                    if (in == socket) {
                        connected = true;
                        break;
                    }
                }
                if(connected) break;
            }
        }
    }

    if(connected) {
        WorkerThread::update(UpdateInfo(this, node));
    }
}

DoutSocket* Viewer::getStart()
{
    return start;
}

void Viewer::setStart(DoutSocket* value)
{
    start = value;
    update_viewer(static_cast<DinSocket*>(nullptr));
}

QWidget* Viewer::getWidget()
{
    return widget;
}

void Viewer::setWidget(QWidget* value)
{
    widget = value;
    if(!widget) std::cout<<"no valid viewer widget" << std::endl;
}

void Viewer::cacheAndUpdate()
{
    dataCache.start(start);
    if(_settingsNode)
        settingsCache.start(_settingsNode->getOutSockets()[0]);
    MT_CUSTOM_SIGNAL_EMITTER("STATUSUPDATE", std::string("done updating"));
    update();
}

DNode* Viewer::getSettings()
{
    return _settingsNode.get();
}
