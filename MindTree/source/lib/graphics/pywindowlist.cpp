#include "graphics/viewer_dock_base.h"
#include "graphics/windowlist.h"
#include "data/python/wrapper.h"
#include "data/python/pyutils.h"
#include "data/nodes/data_node_socket.h"
#include "pywindowlist.h"

using namespace MindTree;
using namespace MindTree::Python;

PythonWindowFactory::PythonWindowFactory(QString name, BPy::object cls)
    : WindowFactory(name), cls(cls)
{
}

PythonWindowFactory::~PythonWindowFactory()
{
}

QString PythonWindowFactory::showWindow()    
{
    auto dock = createWindow();
    MindTree::WindowList::instance()->showDock(dock);
    return dock->objectName();
}

MindTree::ViewerDockBase* PythonWindowFactory::createWindow()    
{
    ViewerDockBase *viewer = new ViewerDockBase(getName());
    viewer->setPythonWidget(cls);
    return viewer;
}

PythonViewerFactory::PythonViewerFactory(QString name, QString type, BPy::object cls)
    : ViewerFactory(name, type), cls(cls)
{
}

PythonViewerFactory::~PythonViewerFactory()
{
}

MindTree::ViewerDockBase* PythonViewerFactory::createViewer(DoutSocket *socket)    
{
    std::cout<<"creating Python Viewer"<<std::endl;
    ViewerDockBase *dock = new ViewerDockBase(getName());
    PyViewerBase *base = nullptr;
    {
        GILLocker locker;
        BPy::object obj = cls(new DoutSocketPyWrapper(socket));
        base = BPy::extract<PyViewerBase*>(obj);
    }
    dock->setViewer(base);
    dock->setStart(socket);
    return dock;
}

PyViewerBase::PyViewerBase(DoutSocketPyWrapper *start)
    : Viewer(start->getWrapped<DoutSocket>())
{
}

PyViewerBase::~PyViewerBase()
{
}

void PyViewerBase::wrap()
{
    BPy::class_<PyViewerBase, boost::noncopyable>("Viewer", BPy::init<DoutSocketPyWrapper*>())
        .def("setWidget", &PyViewerBase::setWidget)
        .add_property("cache", &PyViewerBase::getCache)
        .add_property("socket", 
                      BPy::make_function(&PyViewerBase::getSocket, 
                                         BPy::return_value_policy<BPy::manage_new_object>()));
}

DoutSocketPyWrapper* PyViewerBase::getSocket()
{
    GILReleaser releaser;
    return new DoutSocketPyWrapper(getStart());
}

DataCache PyViewerBase::getCache() const
{
    GILReleaser releaser;
    return cache;
}

void PyViewerBase::update()
{
    GILLocker locker;
    std::cout<<"updating python viewer ... " << std::endl;
    if(auto f = get_override("update"))
        f();
}

void PyViewerBase::setWidget(BPy::object pywidget)
{
    widget = pywidget;
    WId id = BPy::extract<WId>(widget.attr("winId")());
    auto *w = QWidget::find(id);
    if(w) Viewer::setWidget(w);
    else std::cout<<"Python Viewer not valid"<<std::endl;
}

void MindTree::Python::wrapViewerFunctions()
{
    BPy::scope MT;
    BPy::object guiModule(BPy::borrowed(PyImport_AddModule("gui")));
    {
        MT.attr("gui") = guiModule;
        BPy::scope guiscope = guiModule;

        BPy::def("registerWindow", MindTree::Python::regWindow);
        BPy::def("getViewers", MindTree::Python::getViewers);
        BPy::def("getWindows", MindTree::Python::getWindows);
        BPy::def("showViewer", MindTree::Python::showViewer);
        BPy::def("showWindow", MindTree::Python::showWindow);
        BPy::def("showSplitWindow", MindTree::Python::showSplitWindow);
        BPy::def("showTabbedWindow", MindTree::Python::showTabbedWindow);
        BPy::def("registerViewer", MindTree::Python::regViewer);
    }
}

QString MindTree::Python::showWindow(QString name)    
{
    return MindTree::WindowList::instance()->showWindow(name);    
}

QString MindTree::Python::showSplitWindow(QString name, QString other, QString orientation, float ratio)    
{
    Qt::Orientation orient;
    if(orientation == "HORIZONTAL"){
        orient = Qt::Horizontal;
    }
    else if(orientation == "VERTICAL"){
        orient = Qt::Vertical;
    }
    else {
        std::cout << "unknown orientation: " << orientation.toStdString() << std::endl;
        return "";
    }

    return MindTree::WindowList::instance()->showSplitWindow(name, other, orient, ratio);
}

QString MindTree::Python::showTabbedWindow(QString name, QString other)    
{
    return MindTree::WindowList::instance()->showTabbedWindow(name, other);    
}

void MindTree::Python::regWindow(QString name, BPy::object windowClass)    
{
    try {
        //BPy::object cls(BPy::handle<>(BPy::borrowed(windowClass)));
        //BPy::object cls(BPy::handle<>(windowClass));
        if(!MindTree::WindowList::instance()->isRegistered(name)) {
            PythonWindowFactory *factory = new PythonWindowFactory(name, windowClass);
            MindTree::WindowList::instance()->addFactory(factory);
        }
        else
            std::cout<<"Window " << name.toStdString() << "already registered" << std::endl;
    } catch(BPy::error_already_set const &) {
        PyErr_Print();
    }
}

void MindTree::Python::regViewer(QString name, QString type, BPy::object viewerClass)    
{
    try {
        if(!MindTree::ViewerList::instance()->isRegistered(name.toStdString(), type.toStdString())) {
            PythonViewerFactory *factory = new PythonViewerFactory(name, type, viewerClass);
            MindTree::ViewerList::instance()->addViewer(factory);
        }
        else
            std::cout<<"Viewer " << name.toStdString() << "already registered" << std::endl;
    } catch(BPy::error_already_set const &) {
        PyErr_Print();
    }
}

BPy::list MindTree::Python::getWindows()    
{
    GILReleaser releaser;
    BPy::list window_types;
    for(auto &fac : MindTree::WindowList::instance()->getFactories()){
        window_types.append(fac->getName());
    }
    return window_types;
}

BPy::dict MindTree::Python::getViewers()    
{
    GILReleaser releaser;
    BPy::dict viewer_types;
    for(auto &vectors : MindTree::ViewerList::instance()->getFactories()){
        BPy::list viewers;
        for(auto &v : vectors.second){
            viewers.append<QString>(v->getName());
        }
        viewer_types[vectors.first] = viewers;
    }
    return viewer_types;
}

QString MindTree::Python::showViewer(DoutSocketPyWrapper *pyout, unsigned int index)    
{
    if(!pyout->alive()) return "";
    {
        GILReleaser releaser;
        return MindTree::ViewerList::instance()->showViewer(pyout->getWrapped<DoutSocket>(), index); 
    }
}

