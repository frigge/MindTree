#include "data/nodes/node_db.h"
#include "pyutils.h"
#include "data/signal.h"
#include "data/nodes/nodetype.h"
#include "data/nodes/data_node_socket.h"
#include "data/project.h"

#include "boost/python/suite/indexing/vector_indexing_suite.hpp"
#include "system.h"

using namespace MindTree::Python::sys;

void MindTree::Python::sys::wrap()
{
    BPy::def("registerNode", regNode);
    BPy::def("createNode", createNode);
    BPy::def("getRegisteredNodes", getRegisteredNodes);

    BPy::class_<std::vector<std::string>>("StringList")
        .def(BPy::vector_indexing_suite<std::vector<std::string>>())
        .def("__str__", MindTree::Python::sys::__str__StringVector)
        .def("__repr__", MindTree::Python::sys::__repr__StringVector);

    BPy::def("attachToSignal", MindTree::Python::sys::attachToSignal);
    BPy::def("attachToBoundSignal", MindTree::Python::sys::attachToBoundSignal);
    BPy::def("openProject", MindTree::Python::sys::open);
    BPy::def("newProject", MindTree::Python::sys::newProject);

    MindTree::Signal::getHandler<MindTree::Project*>().connect("newProject", [](MindTree::Project* prj) {
        GILLocker locker;
        BPy::object mtmodule = BPy::import("MT");
        ProjectPyWrapper *project = new ProjectPyWrapper(prj);
        BPy::scope(mtmodule).attr("project") = BPy::ptr(project);
    }).detach();
}

void MindTree::Python::sys::open(std::string filename)
{
    MindTree::Project::load(filename);
}

void MindTree::Python::sys::newProject()
{
    MindTree::Project::create();
}

void MindTree::Python::sys::regNode(PyObject *nodeClass)    
{
    try {
        BPy::object cls(BPy::handle<>(BPy::borrowed(nodeClass)));
        NodeDataBase::registerNodeType(std::make_unique<PythonNodeDecorator>(cls));
    } catch(BPy::error_already_set const &) {
        PyErr_Print();
    }
}

BPy::object MindTree::Python::sys::createNode(std::string name)    
{
    GILReleaser releaser;
    NodePtr node = MindTree::NodeDataBase::createNode(name);
    if(!node) return BPy::object();

    return utils::getPyObject(node);
}

BPy::list MindTree::Python::sys::getRegisteredNodes()    
{
    BPy::list pylist;
    for(AbstractNodeDecorator *fac : NodeDataBase::getFactories()){
        pylist.append(fac->getLabel());
    }
    return pylist;
}

MindTree::Signal::CallbackHandler MindTree::Python::sys::attachToSignal(std::string id, BPy::object fn)    
{
    try{
        auto handler = MindTree::Signal::getHandler<BPy::object>().connect(id, fn); 
        return handler;
    } catch(BPy::error_already_set const &){
        PyErr_Print();
    }
    return MindTree::Signal::CallbackHandler();
}

MindTree::Signal::CallbackHandler MindTree::Python::sys::attachToBoundSignal(BPy::object livetime, std::string id, BPy::object fn)
{
    try {
        PyWrapper *wrapper = BPy::extract<PyWrapper*>(livetime);

        void* addr = wrapper->getWrapped<void>();

        auto &handler = MindTree::Signal::getBoundHandler<BPy::object>(addr);
        return handler.connect(id, fn);

    } catch(BPy::error_already_set const &) {
        PyErr_Print();
    }
    return MindTree::Signal::CallbackHandler();
}

std::vector<std::string> MindTree::Python::sys::getNodeTypes()    
{
    return std::vector<std::string>();
}

std::vector<std::string> MindTree::Python::sys::getSocketTypes()    
{
    return std::vector<std::string>();
}

std::string MindTree::Python::sys::__str__StringVector(std::vector<std::string> &self)    
{
    std::string str("[");
    for(auto s : self){
        str += ", " + s;
    }
    str += "]";

    return str;
}

std::string MindTree::Python::sys::__repr__StringVector(std::vector<std::string> &self)    
{
    return "<" + __str__StringVector(self) + ">";
}
