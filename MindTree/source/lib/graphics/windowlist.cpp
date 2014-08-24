#include "data/nodes/data_node_socket.h"
#include "data/windowfactory.h"
#include "graphics/viewer_dock_base.h"

#include "iostream"
#include "QMainWindow"

#include "windowlist.h"

MindTree::WindowList *MindTree::WindowList::windowList = 0;

MindTree::WindowList::WindowList()
{
}

MindTree::WindowList* MindTree::WindowList::instance()
{
    if(!windowList) windowList = new WindowList();
    return windowList;
}

void MindTree::WindowList::setMainWindow(QMainWindow *mw)    
{
    window = mw;
}

QList<MindTree::WindowFactory*> MindTree::WindowList::getFactories()
{
    return windowFactories;
}

bool MindTree::WindowList::isRegistered(QString name)
{
    for(auto fac : windowFactories){
        if (fac->getName() == name) return true;
    }
    return false;
}

void MindTree::WindowList::addFactory(MindTree::WindowFactory* value)
{
    windowFactories.append(value);
    Q_EMIT windowFactoryAdded(value);
}

MindTree::ViewerDockBase* MindTree::WindowList::createWindow(QString name)    
{
    for(auto fac : windowFactories){
        if(fac->getName() == name) {
            return fac->createWindow();
        }
    }
    return nullptr;
}

QString MindTree::WindowList::showWindow(QString name)    
{
    auto dock = createWindow(name);
    if(!dock) return "";
    window->addDockWidget(Qt::LeftDockWidgetArea, dock);
    return dock->objectName();
}

QString MindTree::WindowList::showSplitWindow(QString name, QString other, Qt::Orientation orientation, float ratio)    
{
    auto dock = createWindow(name);
    if(!dock) return "";
    auto otherDock = window->findChild<QDockWidget*>(other);
    int width = otherDock->width();
    window->splitDockWidget(otherDock, dock, orientation);
    dock->resize(width/ratio, dock->height());
    return dock->objectName(); 
}

QString MindTree::WindowList::showTabbedWindow(QString name, QString other)    
{
    auto dock = createWindow(name);
    if(!dock) return "";
    auto otherDock = window->findChild<QDockWidget*>(other);
    window->tabifyDockWidget(otherDock, dock);
    return dock->objectName();
}

void MindTree::WindowList::showDock(QDockWidget *widget)
{
    window->addDockWidget(Qt::LeftDockWidgetArea, widget);
}

MindTree::ViewerList *MindTree::ViewerList::viewerList = 0;

MindTree::ViewerList::ViewerList()
{
}

void MindTree::ViewerList::setMainWindow(QMainWindow *mw)    
{
    window = mw;
}

MindTree::ViewerList* MindTree::ViewerList::instance()    
{
    if(!viewerList) viewerList = new ViewerList();
    return viewerList;
}

std::map<std::string, std::vector<MindTree::ViewerFactory*>> MindTree::ViewerList::getFactories()
{
    return windowFactories;
}

bool MindTree::ViewerList::isRegistered(std::string name, std::string type)
{
    if(windowFactories.count(type) < 1) return false;
    for(auto v : windowFactories[type])
        if(v->getName().toStdString() == name) return true;
    return false;
}

void MindTree::ViewerList::addViewer(MindTree::ViewerFactory* value)
{
    if(windowFactories.count(value->getType().toStdString()))
        windowFactories[value->getType().toStdString()].push_back(value);
    else {
        auto vec = {value};
        windowFactories.insert(std::make_pair(value->getType().toStdString(), vec));
    }
}

void MindTree::ViewerList::showDock(QDockWidget* dock)
{
    window->addDockWidget(Qt::LeftDockWidgetArea, dock);
}

QString MindTree::ViewerList::showViewer(DoutSocket *socket, unsigned int index)
{
    std::cout<< "show the viewer" << std::endl;
    if(!windowFactories.count(socket->getType().toStr())) {
        std::cout<<"No Viewer registered for this type"<<std::endl;
        return "";
    }

    std::string typestr = socket->getType().toStr();
    std::string idstr = typestr + std::to_string(index);
    ViewerDockBase *dock = nullptr;
    if(openViewers.find(idstr) == openViewers.end()) {
        dock = windowFactories[typestr][index]->createViewer(socket);
        openViewers.insert({typestr + std::to_string(index), dock});
        showDock(dock);
    } else {
        dock = openViewers[idstr];  
        dock->setStart(socket);
    }

    return dock->objectName();
}

