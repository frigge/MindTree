#include "data/python/wrapper.h"
#include "data/signal.h"
#include "data/nodes/data_node.h"
#include "QWidget"
#include "viewer.h"

using namespace MindTree;

Viewer::Viewer(DoutSocket *start)
    : widget(0),
    start(start),
    _needToUpdate(false),
    _running(true)
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

    auto updateFunc = [this]{
        while(this->running()) {
            if(this->needToUpdate()) {
                this->cache.start(this->start);
                {
                    std::lock_guard<std::mutex> lock(_updateMutex);
                    this->_needToUpdate = false;
                }
                this->update();
            }
        }
    };

    _updateThread = std::thread(updateFunc);
}

Viewer::~Viewer()
{
    {
        std::lock_guard<std::mutex> lock(_runningMutex);
        _running = false;
    }
    _updateThread.join();
}

bool Viewer::running()
{
    std::lock_guard<std::mutex> lock(_runningMutex);
    return _running;
}

bool Viewer::needToUpdate()
{
    std::lock_guard<std::mutex> lock(_updateMutex);
    return _needToUpdate;
}

void Viewer::notifyUpdate()
{
    std::lock_guard<std::mutex> lock(_updateMutex);
    _needToUpdate = true;
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
        notifyUpdate();
    }
}

void Viewer::update_viewer(DinSocket *socket)    
{
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
        notifyUpdate();
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

