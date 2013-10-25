#include "data/signal.h"
#include "QGraphicsItem"
#include "gui.h"


bool NodeDesigner::initialized=false;
//std::unordered_map<MindTree::NodeType, NodeDesigner> NodeDesigner::designers;

NodeDesigner::NodeDesigner(BPy::object designer)
    : nodeDesigner(designer)
{
}

NodeDesigner::NodeDesigner(const NodeDesigner &other)
    :nodeDesigner(other.nodeDesigner)
{
}

NodeDesigner::~NodeDesigner()
{
}

void NodeDesigner::registerDesigner(QString type, NodeDesigner designer)    
{
    MindTree::NodeType t(type.toStdString());
    //designers.insert(std::pair<MindTree::NodeType, NodeDesigner>(t, designer));
}

void NodeDesigner::removeDesigner(QString type)    
{
    MindTree::NodeType t(type.toStdString());
}

//QGraphicsItem* NodeDesigner::create(MindTree::NodeType t)    
//{
//    //NodeDesigner designer(designers.at(t));
//    //return designer();
//}

QGraphicsItem* NodeDesigner::operator()()    
{
   return BPy::extract<QGraphicsItem*>(nodeDesigner()); 
}

void NodeDesigner::exposeToPython()    
{
    BPy::scope MT;
    BPy::object designerModule(BPy::borrowed(PyImport_AddModule("nodeDesigner")));
    try {
        MT.attr("nodeDesigner") = designerModule;
    }
    catch (BPy::error_already_set const &) {
        PyErr_Print();
    }
    BPy::scope subscope = designerModule;

    BPy::def("registerDesigner", registerDesigner);
    BPy::def("removeDesigner", removeDesigner);
    initialized=true;
    std::cout<< "initialized NodeDesigner Python bindings" << std::endl;
}

void NodeDesigner::init()    
{
    if(initialized) return;
    MindTree::Signal::getHandler<>().add("void PyMT::wrap_all()", exposeToPython).detach();     

}
