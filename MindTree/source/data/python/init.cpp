/*
    FRG Shader Editor, a Node-based Renderman Shading Language Editor
    Copyright (C) 2011  Sascha Fricke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "data/python/wrapper.h"
#include "data/project.h"
#include "QApplication"
#include "QDir"
#include "iostream"
#include "string"

#include "boost/locale.hpp"

#include "data/signal.h"
#include "graphics/viewer.h"
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"
#include "data/python/pycache_main.h"
#include "graphics/viewer.h"
#include "graphics/pywindowlist.h"
#include "data/python/pyutils.h"
#include "data/python/system.h"

#include "init.h"

using namespace MindTree;
using namespace MindTree::Python;

std::string MindTree::Python::type(const BPy::object &obj)
{
    return BPy::extract<std::string>(obj.attr("__class__").attr("__name__"));
}

int getGraphicsCount()
{
    int cnt = 0;
    //foreach(QGraphicsItem *item, FRG::Space->items())
    //    if(item->type() == VNode::Type) cnt++;
    return cnt;
}

BOOST_PYTHON_MODULE(MT){
    BPy::object main = BPy::import("__main__");
    BPy::object global = main.attr("__dict__");

    BPy::def("unregisterNodeType", NodeType::unregisterType);

    MindTree::Python::wrapViewerFunctions();
#ifdef QT_DEBUG
    BPy::def("getStylePath", PyWrapper::getStylePath);
    BPy::def("setStylePath", PyWrapper::setStylePath);
#endif
    sys::wrap();

    wrap_all();
}

void MindTree::Python::wrap_all()
{
    BPy::scope MT;
    BPy::object typeModule(BPy::borrowed(PyImport_AddModule("pytypes")));
    {
        MT.attr("pytypes") = typeModule;
        BPy::scope typeScope = typeModule;

        BPy::class_<MindTree::Signal::CallbackHandler>("CallbackHandler", BPy::no_init)
            .def("detach", &MindTree::Signal::CallbackHandler::detach)
            .def("destruct", &MindTree::Signal::CallbackHandler::destruct);
        PyWrapper::wrap();
        ProjectPyWrapper::wrap();
        DNSpacePyWrapper::wrap();
        DNodeListIteratorPyWrapper::wrap();
        DNodePyWrapper::wrap();
        DSocketPyWrapper::wrap();
        DinSocketPyWrapper::wrap();
        DoutSocketPyWrapper::wrap();
        PyViewerBase::wrap();
        ContainerNodePyWrapper::wrap();
        ContainerSpacePyWrapper::wrap();
        PropertyPyWrapper::wrap();
        MindTree::Signal::SignalHandler<>::handler("registerPyDataTypes");
    }

    BPy::object pycache_module(BPy::borrowed(PyImport_AddModule("cache")));
    {
        MT.attr("cache") = pycache_module;
        BPy::scope pycache_scope = pycache_module;
        wrap_DataCache();
    }

    MindTree::Signal::SignalHandler<>::handler(__PRETTY_FUNCTION__);
}

PyObject* QPointFToPython::convert(QPointF const &pos)
{
    BPy::tuple t = BPy::make_tuple(pos.x(), pos.y());
    return BPy::incref(t.ptr());
}

PyObject* QStringToPython::convert(QString const &s)
{
    BPy::object string(s.toStdString());
    return BPy::incref(string.ptr());
}

PythonToQString::PythonToQString()
{
    BPy::converter::registry::push_back(&convertible, &construct, BPy::type_id<QString>());
}

void* PythonToQString::convertible(PyObject* obj)
{
    if(!PyUnicode_Check(obj)) return 0;
    return obj;
}

void PythonToQString::construct(PyObject *obj, BPy::converter::rvalue_from_python_stage1_data *data)    
{
    BPy::object strobj(BPy::handle<>(BPy::borrowed(obj)));
    std::string str = BPy::extract<std::string>(strobj);
    void *storage = ((BPy::converter::rvalue_from_python_storage<QString>*)data)->storage.bytes;
    new (storage) QString(str.c_str());
    data->convertible = storage;
}

void MindTree::Python::loadPlugins()
{
    MindTree::Python::GILLocker locker;
    QString dir("../plugins");
    QDir plugDir(dir);
    if(!plugDir.exists()) {
        std::cout << "Plugin Directory not found: " << plugDir.absolutePath().toStdString() << std::endl;
        return;
    }

    BPy::import("sys").attr("path").attr("append")(plugDir.absolutePath().toStdString());

    for(QString plugin : plugDir.entryList()){
        if(plugin == "." || plugin == "..") continue;
        load(dir, plugin);
    }
}

void MindTree::Python::loadIntern()
{
    MindTree::Python::GILLocker locker;
    QString dir("../intern");
    QDir plugDir(dir);
    if(!plugDir.exists()) {
        std::cout << "Internal Directory not found: " << plugDir.absolutePath().toStdString() << std::endl;
        return;
    }

    BPy::import("sys").attr("path").attr("append")(plugDir.absolutePath().toStdString());

    for(QString plugin : plugDir.entryList()){
        if(plugin == "." || plugin == "..") continue;
        load(dir, plugin);
    }
}

void MindTree::Python::loadSettings()
{
    MindTree::Python::GILLocker locker;
    QString dir("../settings");
    QDir plugDir(dir);
    if(!plugDir.exists()) {
        std::cout << "Settings Directory not found: " << plugDir.absolutePath().toStdString() << std::endl;
        return;
    }

    BPy::import("sys").attr("path").attr("append")(plugDir.absolutePath().toStdString());

    for(QString plugin : plugDir.entryList()){
        if(plugin == "." || plugin == "..") continue;
        load(dir, plugin);
    }
}

void MindTree::Python::load(QString path, QString plugin)
{
    bool is_python_module = QFileInfo(path + "/" + plugin).isDir() || plugin.endsWith(".py") || plugin.endsWith(".pyc");
    if(!is_python_module) {
        return;
    }
            
    if(plugin.endsWith(".py")){
        plugin = plugin.replace(".py", "");
    }
    else if(plugin.endsWith(".pyc")){
        plugin = plugin.replace(".pyc", "");
    }
    try{
        BPy::import(BPy::str(plugin.toStdString()));
    }catch(BPy::error_already_set const &){
        std::cout << "could not load " << plugin.toStdString() << std::endl;
        PyErr_Print();
    }
}

void MindTree::Python::init(int argc, char *argv[])
{
    PyImport_AppendInittab("MT", &PyInit_MT);
    Py_Initialize();
    PyEval_InitThreads();

    //wchar_t** dest;
    //for (int i = 0; i < argc; i++) {
    //    dest[i] = new wchar_t[strlen(argv[i]) + 1];
    //    mbtowc(dest[i], argv[i], strlen(argv[i]));
    //}
    //PySys_SetArgv(argc, dest);
    //BPy::to_python_converter<DNSpace*, PointerToPython<DNSpace*> >();
    try{
        BPy::to_python_converter<QString, QStringToPython>();
        BPy::to_python_converter<QPointF, QPointFToPython>();
        BPy::to_python_converter<QList<DNode*>, QListToPython<DNode*> >();
        BPy::to_python_converter<QList<QString>, QListToPython<QString> >();
        BPy::to_python_converter<QList<DoutSocket*>, QListToPython<DoutSocket*> >();
        PythonToQString();
        //initMT();// init_module_MT();
        BPy::object main = BPy::import("__main__");
        BPy::object global = main.attr("__dict__");
        BPy::object mtmodule = BPy::import("MT");
        global["MT"] = mtmodule;
        ProjectPyWrapper *project = new ProjectPyWrapper(Project::instance());
        BPy::scope(mtmodule).attr("project") = BPy::ptr(project);
        BPy::scope(mtmodule).attr("signalIDs") = BPy::ptr(&MindTree::Signal::emitterIDs);
    }catch(BPy::error_already_set const &){
        PyErr_Print();
    }
    // Release the GIL from the main thread initially => acquire it explicitly always
    /*  PyThreadState *state = */PyEval_SaveThread();
}

void MindTree::Python::finalize()    
{
    PyGILState_Ensure();
    Py_Finalize();
}

