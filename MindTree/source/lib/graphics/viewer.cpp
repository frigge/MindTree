#include "data/python/wrapper.h"
#include "data/signal.h"
#include "data/nodes/data_node.h"
#include "QWidget"
#include "viewer.h"

using namespace MindTree;

Viewer::Viewer(DoutSocket *start)
    : start(start), widget(0)
{
    Signal::getHandler<DinSocket*>()
        .add("createLink", 
                std::bind(&Viewer::update_viewer, this, std::placeholders::_1)).detach();
    Signal::getHandler<DinSocket*>()
        .add("socketChanged", 
                std::bind(&Viewer::update_viewer, this, std::placeholders::_1)).detach();
}

Viewer::~Viewer()
{
}

void Viewer::update_viewer(DinSocket *socket)    
{
    update(socket);
}

void Viewer::update(DinSocket*)    
{
    std::cout<<"updating viewer ... " << std::endl;
}

DoutSocket* Viewer::getStart()
{
    return start;
}

void Viewer::setStart(DoutSocket* value)
{
    start = value;
}

QWidget* Viewer::getWidget()
{
    return widget;
}

void Viewer::setWidget(QWidget* value)
{
    if(!widget) std::cout<<"no valid viewer widget" << std::endl;
    widget = value;
}

