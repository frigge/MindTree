#include "data/python/wrapper.h"
#include "data/signal.h"
#include "data/nodes/data_node.h"
#include "QWidget"
#include "viewer.h"

using namespace MindTree;

Viewer::Viewer(DoutSocket *start)
    : widget(0),
    start(start)
{
    Signal::getHandler<DinSocket*>()
        .connect("createLink", 
                std::bind(&Viewer::update_viewer, 
                          this, 
                          std::placeholders::_1)).detach();

    Signal::getHandler<DinSocket*>()
        .connect("socketChanged", 
                std::bind(&Viewer::update_viewer, 
                          this, 
                          std::placeholders::_1)).detach();
}

Viewer::~Viewer()
{
}

void Viewer::update_viewer(DinSocket *socket)    
{
    cache.start(start);
    update(socket);
}

void Viewer::update(DinSocket*)    
{
}

DoutSocket* Viewer::getStart()
{
    return start;
}

void Viewer::setStart(DoutSocket* value)
{
    start = value;
    update_viewer(nullptr);
}

QWidget* Viewer::getWidget()
{
    return widget;
}

void Viewer::setWidget(QWidget* value)
{
    widget = value;
    if(!widget) std::cout<<"no valid viewer widget" << std::endl;
    update_viewer(nullptr);
}

