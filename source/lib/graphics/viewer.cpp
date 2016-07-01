#include "data/python/wrapper.h"
#include "data/signal.h"
#include "data/nodes/data_node.h"
#include "data/debuglog.h"

#include "QWidget"

#include "viewer.h"

using namespace MindTree;

std::thread WorkerThread::_updateThread;
std::mutex WorkerThread::_updateMutex;
std::condition_variable WorkerThread::_needToUpdateCondition;
std::atomic<bool> WorkerThread::_running(false);

std::vector<std::weak_ptr<WorkerThread::UpdateInfo>> WorkerThread::_updateQueue;

bool WorkerThread::needToUpdate()
{
    std::lock_guard<std::mutex> lock(_updateMutex);
    return !_updateQueue.empty();
}

void WorkerThread::notifyUpdate(std::weak_ptr<WorkerThread::UpdateInfo> info)
{
    {
        std::lock_guard<std::mutex> lock(_updateMutex);
        _updateQueue.push_back(info);
    }
    if(!_running) start();
}

void WorkerThread::update()
{
    _needToUpdateCondition.notify_all();
}

void WorkerThread::removeViewer(Viewer *viewer)
{
    {
        std::lock_guard<std::mutex> lock(_updateMutex);
        auto it = std::find_if(begin(_updateQueue),
                               end(_updateQueue),
                               [viewer] (std::weak_ptr<UpdateInfo> i) {
                               return i.lock()->_viewer == viewer;
                               });
        if(it != end(_updateQueue))
            _updateQueue.erase(it);
    }

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

            for(auto &info : _updateQueue) {
                if(!info.expired()) {
                    auto ptr = info.lock();
                    if(!ptr->_update) continue;

                    if(ptr->_node) DataCache::invalidate(ptr->_node);
                    ptr->_viewer->cacheAndUpdate();
                    ptr->_update = false;
                }
            }
        }
        WorkerThread::_running = false;
    };

    _updateThread = std::thread(updateFunc);
}

void WorkerThread::stop()
{
    {
        std::lock_guard<std::mutex> lock(_updateMutex);
        _updateQueue.clear();
    }
    _needToUpdateCondition.notify_all();

    if (_updateThread.joinable()) _updateThread.join();
}

Viewer::Viewer(DoutSocket *start)
    : widget(0),
    start(start),
    _signalLiveTime(new Signal::LiveTimeTracker(this)),
    _updateInfo(std::make_shared<WorkerThread::UpdateInfo>(this))
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
    WorkerThread::notifyUpdate(_updateInfo);
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
        _updateInfo->_node = node;
        _updateInfo->_update = true;
        WorkerThread::update();
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

    if(connected) {
        _updateInfo->_node = node;
        _updateInfo->_update = true;
        WorkerThread::update();
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
    update();
    MT_SIGNAL_EMITTER("STATUSUPDATE", std::string("done updating"));
}

DNode* Viewer::getSettings()
{
    return _settingsNode.get();
}
