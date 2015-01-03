#include "data/python/wrapper.h"
#include "data/signal.h"
#include "data/nodes/data_node.h"
#include "data/debuglog.h"

#include "QWidget"

#include "viewer.h"

using namespace MindTree;

std::thread WorkerThread::_updateThread;
std::mutex WorkerThread::_updateMutex;
std::atomic<bool> WorkerThread::_running(false);

std::deque<Viewer*> WorkerThread::_updateQueue;

bool WorkerThread::needToUpdate()
{
    std::lock_guard<std::mutex> lock(_updateMutex);
    return !_updateQueue.empty();
}

void WorkerThread::notifyUpdate(Viewer *viewer)
{
    {
        std::lock_guard<std::mutex> lock(_updateMutex);
        _updateQueue.push_back(viewer);
    }
    if(!_running) start();
}

void WorkerThread::removeViewer(Viewer *viewer)
{
    {
        std::lock_guard<std::mutex> lock(_updateMutex);
        if(std::find(begin(_updateQueue), end(_updateQueue), viewer) != end(_updateQueue))
            _updateQueue.erase(std::remove(begin(_updateQueue), end(_updateQueue), viewer));
    }

    if(!needToUpdate()) stop();
}

void WorkerThread::start()
{
    if(_updateThread.joinable()) _updateThread.join();

    auto updateFunc = []{
        WorkerThread::_running = true;
        while(WorkerThread::needToUpdate()) {
            {
                std::lock_guard<std::mutex> lock(_updateMutex);

                Viewer *viewer = _updateQueue.front();
                viewer->cache.start(viewer->start);
                _updateQueue.pop_front();
                viewer->update();
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

    if (_updateThread.joinable()) _updateThread.join();
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

void Viewer::update_viewer(DNode *node)
{
    if(node) DataCache::invalidate(node);
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
        WorkerThread::notifyUpdate(this);
    }
}

void Viewer::update_viewer(DinSocket *socket)
{
    if(socket) DataCache::invalidate(socket->getNode());
    //check whether start and socket are connected
    bool connected = false;
    if (!socket || socket->getNode() == start->getNode()) {
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
    }

    if(connected) {
        WorkerThread::notifyUpdate(this);
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
    update_viewer(static_cast<DinSocket*>(nullptr));
}

