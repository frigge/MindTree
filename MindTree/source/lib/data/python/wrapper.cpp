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

#include "data/project.h"
#include "data/nodes/data_node.h"
#include "data/nodes/data_node_socket.h"
#include "data/dnspace.h"
#include "pyexposable.h"
#include "data/frg.h"
#include "graphics/windowlist.h"
#include "graphics/viewer_dock_base.h"
#include "graphics/viewer.h"
#include "data/signal.h"
#include "data/properties.h"
#include "wrapper.h"

using namespace MindTree;

DNodeListIteratorPyWrapper::DNodeListIteratorPyWrapper(NodeList nodelist)
    : nodelist(nodelist), iterator(nodelist.begin())
{
}

DNodeListIteratorPyWrapper::~DNodeListIteratorPyWrapper()
{
}

void DNodeListIteratorPyWrapper::wrap()    
{
    BPy::class_<DNodeListIteratorPyWrapper>("NodeIter", BPy::no_init) 
            .def("__iter__", &DNodeListIteratorPyWrapper::iter, BPy::return_value_policy<BPy::manage_new_object>())
            .def("next", &DNodeListIteratorPyWrapper::next, BPy::return_value_policy<BPy::manage_new_object>());
}

DNodeListIteratorPyWrapper* DNodeListIteratorPyWrapper::iter()    
{
    return new DNodeListIteratorPyWrapper(nodelist);
}

DNodePyWrapper* DNodeListIteratorPyWrapper::next()    
{
    if(iterator == nodelist.end()){
        PyErr_SetNone(PyExc_StopIteration);
        BPy::throw_error_already_set();
        return 0;
    }
    DNodePyWrapper* node = new DNodePyWrapper(*iterator);
    iterator++;
    return node;
}

PythonNodeFactory::PythonNodeFactory(BPy::object cls)
    : cls(cls) 
{
    setType(BPy::extract<std::string>(cls.attr("type")));
    setLabel(BPy::extract<std::string>(cls.attr("label")));
}

PythonNodeFactory::~PythonNodeFactory()
{
}

DNode* PythonNodeFactory::operator()()    
{
    DNode *node = new DNode();
    node->setNodeType(getType());
    try{
        cls(new DNodePyWrapper(node));
    } catch (BPy::error_already_set){
        PyErr_Print();
    }
    return node;
}


#ifdef QT_DEBUG
QString MindTree::PyWrapper::stylePath = "../styles.rcc";
#endif

PyWrapper::PyWrapper(PyExposable *exp)
    : element(exp)
{
    if(exp) element->regWrapper(this);
}

PyWrapper::~PyWrapper()
{
    if(element)element->rmWrapper(this);
}

std::string PyWrapper::__str__StringVector(std::vector<std::string> &self)    
{
    std::string str("[");
    for(auto s : self){
        str += ", " + s;
    }
    str += "]";

    return str;
}

std::string PyWrapper::__repr__StringVector(std::vector<std::string> &self)    
{
    return "<" + __str__StringVector(self) + ">";
}

MindTree::Signal::CallbackHandler PyWrapper::attachToSignal(std::string id, BPy::object fn)    
{
    try{
        auto handler = MindTree::Signal::getHandler<BPy::object>().add(id, fn); 
        return handler;
    } catch(BPy::error_already_set const &){
        PyErr_Print();
    }
    return MindTree::Signal::CallbackHandler([]{});
}

void PyWrapper::regNode(PyObject *nodeClass)    
{
    try {
        BPy::object cls(BPy::handle<>(BPy::borrowed(nodeClass)));
        NodeDataBase::registerNodeType(new PythonNodeFactory(cls));
    } catch(BPy::error_already_set const &) {
        PyErr_Print();
    }
}

std::vector<std::string> PyWrapper::getNodeTypes()    
{
    return NodeType::getTypes();
}

std::vector<std::string> PyWrapper::getSocketTypes()    
{
    return SocketType::getTypes();
}

DNodePyWrapper* PyWrapper::createNode(std::string name)    
{
    DNode *node = MindTree::NodeDataBase::createNode(name);
    if(!node) return nullptr;

    return new DNodePyWrapper(node);
}

BPy::list PyWrapper::getRegisteredNodes()    
{
    BPy::list pylist;
    for(AbstractNodeFactory *fac : NodeDataBase::getFactories()){
        pylist.append(fac->getLabel());
    }
    return pylist;
}

#ifdef QT_DEBUG
void PyWrapper::setStylePath(QString path)    
{
    stylePath = path;
}

QString PyWrapper::getStylePath()    
{
    return stylePath;
}
#endif

void PyWrapper::elementDestroyed()    
{
    element = 0;
}

bool PyWrapper::alive()
{
    if(!element){
        PyErr_SetString(PyExc_ReferenceError, "Referenced data is already destoryed");
        return false;
    }
    return true;
}

ProjectPyWrapper::ProjectPyWrapper(Project *project)
    : PyWrapper(project)
{
}

ProjectPyWrapper::~ProjectPyWrapper()
{
}

void ProjectPyWrapper::wrap()    
{
   BPy::class_<ProjectPyWrapper>("Project", BPy::no_init)
            .add_property("filename", &ProjectPyWrapper::getFilename, 
                    &ProjectPyWrapper::setFilename)
            .add_property("root", 
                    BPy::make_function(&ProjectPyWrapper::getRoot, 
                        BPy::return_value_policy<BPy::manage_new_object>()));
}

std::string ProjectPyWrapper::getFilename()    
{
    if(!alive()) return std::string();
    return getWrapped<Project>()->getFilename();
}

void ProjectPyWrapper::setFilename(std::string name)    
{
    if(!alive()) return;
    getWrapped<Project>()->setFilename(name);
}

DNSpacePyWrapper* ProjectPyWrapper::getRoot()    
{
    if(!alive()) return 0;
    return new DNSpacePyWrapper(getWrapped<Project>()->getRootSpace());
}

DNSpacePyWrapper::DNSpacePyWrapper(DNSpace *space)
    : PyWrapper(space)
{
}

DNSpacePyWrapper::~DNSpacePyWrapper()
{
}

void DNSpacePyWrapper::wrap()    
{
    DNodePyWrapper*(DNSpacePyWrapper::*getItemString)(std::string) = &DNSpacePyWrapper::__getitem__;
    DNodePyWrapper*(DNSpacePyWrapper::*getItemInt)(int) = &DNSpacePyWrapper::__getitem__;
    BPy::class_<DNSpacePyWrapper>("DNSpace", BPy::no_init)
                .def("addNode", &DNSpacePyWrapper::addNode)
                .def("removeNode", &DNSpacePyWrapper::removeNode)
                .def("isContainer", &DNSpacePyWrapper::isContainer)
                .def("setName",  &DNSpacePyWrapper::setName)
                .def("getName", &DNSpacePyWrapper::getName)
                .def("__str__", &DNSpacePyWrapper::__str__)
                .def("__repr__", &DNSpacePyWrapper::__repr__)
                .def("__iter__", &DNSpacePyWrapper::iter, BPy::return_value_policy<BPy::manage_new_object>())
                .def("__getitem__", getItemString, BPy::return_value_policy<BPy::manage_new_object>())
                .def("__getitem__", getItemInt, BPy::return_value_policy<BPy::manage_new_object>());
}

void DNSpacePyWrapper::addNode(DNodePyWrapper *node)    
{
    if(!(alive() && node->alive())) return;
    getWrapped<DNSpace>()->addNode(node->getWrapped<DNode>());
}

void DNSpacePyWrapper::removeNode(DNodePyWrapper *node)    
{
    if(!(alive() && node->alive())) return;
    getWrapped<DNSpace>()->removeNode(node->getWrapped<DNode>());
}

bool DNSpacePyWrapper::isContainer()    
{
    if(!alive()) return false;
    return getWrapped<DNSpace>()->isContainerSpace();
}

void DNSpacePyWrapper::setName(std::string name)    
{
    if(!alive()) return;
    getWrapped<DNSpace>()->setName(name);
}

std::string DNSpacePyWrapper::getName()    
{
    if(!alive()) return std::string();
    return getWrapped<DNSpace>()->getName();
}

std::string DNSpacePyWrapper::__str__()    
{
    if(!alive()) return std::string();
    std::string str("[");
    for(DNode *node : getWrapped<DNSpace>()->getNodes()){
        str.append("\""+std::string(node->getNodeName().c_str())+"\"");
        if(node != *getWrapped<DNSpace>()->getNodes().end())
            str.append(", ");
    }
    str.append("]");
    return str;
}

std::string DNSpacePyWrapper::__repr__()    
{
    if(!alive()) return "";
    return "<"+getName()+__str__()+">";
}

DNodePyWrapper* DNSpacePyWrapper::__getitem__(std::string name)    
{
    if(!alive()) return 0;
    for(DNode *node : getWrapped<DNSpace>()->getNodes()){
        if(node->getNodeName() == name)
            return new DNodePyWrapper(node);
    }
    PyErr_SetString(PyExc_KeyError, name.c_str());
    PyErr_Print();
    return 0;
}

DNodePyWrapper* DNSpacePyWrapper::__getitem__(int index)    
{
    if(!alive()) return 0;
    if(getWrapped<DNSpace>()->getNodes().size() <= index) {
        PyErr_SetString(PyExc_IndexError, "list index out of range");
        PyErr_Print();
        return 0;
    }
    return new DNodePyWrapper(getWrapped<DNSpace>()->getNodes().at(index));
}

DNodeListIteratorPyWrapper* DNSpacePyWrapper::iter()    
{
    if(!alive()) return 0;
    return new DNodeListIteratorPyWrapper(getWrapped<DNSpace>()->getNodes());
}

DNodePyWrapper::DNodePyWrapper(DNode *node)
    : PyWrapper(node)
{
}

DNodePyWrapper::~DNodePyWrapper()
{
}

void DNodePyWrapper::wrap()    
{
    BPy::class_<DNodePyWrapper>("DNode", BPy::no_init)
                .def("__dir__", &DNodePyWrapper::dir)
                //.def("__setattr__", &DNodePyWrapper::setattr)
                //.def("__getattr__", &DNodePyWrapper::getattr)
                .add_property("name", &DNodePyWrapper::getName, &DNodePyWrapper::setName)
                .add_property("type", &DNodePyWrapper::getType, &DNodePyWrapper::getType)
                .add_property("pos", &DNodePyWrapper::getPos, &DNodePyWrapper::setPos)
                .add_property("insockets", 
                        BPy::make_function(&DNodePyWrapper::in, 
                            BPy::return_value_policy<BPy::manage_new_object>()))
                .add_property("space", 
                        BPy::make_function(&DNodePyWrapper::getSpace, 
                            BPy::return_value_policy<BPy::manage_new_object>()))
                .def("addInSocket", &DNodePyWrapper::addInSocket)
                .def("addOutSocket", &DNodePyWrapper::addOutSocket)
                .add_property("selected", &DNodePyWrapper::getSelected, &DNodePyWrapper::setSelected)
                .add_property("outsockets", 
                        BPy::make_function(&DNodePyWrapper::out, 
                            BPy::return_value_policy<BPy::manage_new_object>()));
}

bool DNodePyWrapper::getSelected()
{
    if(!alive()) return false;
    return getWrapped<DNode>()->getSelected();
}

void DNodePyWrapper::setSelected(bool value)
{
    if(!alive())return;
    getWrapped<DNode>()->setSelected(value);
}

BPy::list DNodePyWrapper::dir(BPy::object self)    
{
    DNodePyWrapper *node_wrapper = BPy::extract<DNodePyWrapper*>(self);
    BPy::list l;
    if(!node_wrapper->alive()) return l;
    auto node = node_wrapper->getWrapped<DNode>();
    l = BPy::dict(self.attr("__dict__")).keys();
    for(auto prop : node->getProperties())
        l.append(prop.first);
    return l;
}

void DNodePyWrapper::setattr(BPy::object self, BPy::object name, BPy::object value)    
{
    DNodePyWrapper *node = 0;
    node = BPy::extract<DNodePyWrapper*>(self);
    if(!node->alive())return;
    std::string n = BPy::extract<std::string>(name);
    auto prop = Property::createPropertyFromPython(n, value);
    node->getWrapped<DNode>()->setProperty(prop);
}

BPy::object DNodePyWrapper::getattr(BPy::object self, std::string key)    
{
    DNodePyWrapper *node_wrapper = BPy::extract<DNodePyWrapper*>(self);
    BPy::list l;
    if(!node_wrapper->alive()) return l;
    auto node = node_wrapper->getWrapped<DNode>();
    auto properties = node->getProperties();

    auto it = properties.find(key);
    if(it != properties.end())
        return (*it).second.toPython();
    
    return self.attr("__dict__")[key];
}

void DNodePyWrapper::addInSocket(std::string name, std::string type)    
{
    if(!alive()) return; 
    new DinSocket(name, SocketType(type), getWrapped<DNode>());
}

void DNodePyWrapper::addOutSocket(std::string name, std::string type)    
{
    if(!alive()) return;
    new DoutSocket(name, SocketType(type), getWrapped<DNode>());
}

DNSpacePyWrapper* DNodePyWrapper::getSpace()    
{
    if(!alive())return 0;
    return new DNSpacePyWrapper(getWrapped<DNode>()->getSpace());
}

void DNodePyWrapper::setName(std::string name)    
{
    if(!alive())return;
    getWrapped<DNode>()->setNodeName(name);
}

std::string DNodePyWrapper::getName()    
{
    if(!alive())return std::string();
    return getWrapped<DNode>()->getNodeName();
}

std::string DNodePyWrapper::getType()
{
    if(!alive())return std::string("");
    auto type = getWrapped<DNode>()->getType();
    return type.toStr();
}

void DNodePyWrapper::setType(std::string value)
{
    if(!alive())return;
    getWrapped<DNode>()->setType(value);
}

BPy::tuple DNodePyWrapper::getPos()    
{
    if(!alive())return BPy::make_tuple(0, 0);
    Vec2i pos = getWrapped<DNode>()->getPos();

    return BPy::make_tuple(pos.x(), pos.y());
}

void DNodePyWrapper::setPos(BPy::tuple pos)
{
    if(!alive())return;
    float x = BPy::extract<float>(pos[0]);
    float y = BPy::extract<float>(pos[1]);
    getWrapped<DNode>()->setPos(Vec2i(x, y));
}

BPy::object DNodePyWrapper::getProperty(std::string name)    
{
    if(!alive())return BPy::object();
    auto prop = getWrapped<DNode>()->getProperty(name);
    return prop.toPython();
}

DSocketListPyWrapper* DNodePyWrapper::in()    
{
    if(!alive()) return 0;
    return new DSocketListPyWrapper(getWrapped<DNode>()->getInSocketLlist());
}

DSocketListPyWrapper* DNodePyWrapper::out()    
{
    if(!alive())return 0;
    return new DSocketListPyWrapper(getWrapped<DNode>()->getOutSocketLlist());
}

DSocketListPyWrapper::DSocketListPyWrapper(DSocketList *list)
    : PyWrapper(list)
{
}

DSocketListPyWrapper::~DSocketListPyWrapper()
{
}

void DSocketListPyWrapper::wrap()    
{
    BPy::class_<DSocketListPyWrapper>("DSocketList", BPy::no_init)
                .def("__str__", &DSocketListPyWrapper::__str__)
                .def("__repr__", &DSocketListPyWrapper::__repr__)
                .def("__getitem__", &DSocketListPyWrapper::getitemint, BPy::return_value_policy<BPy::manage_new_object>())
                .def("__getitem__", &DSocketListPyWrapper::getitemstr, BPy::return_value_policy<BPy::manage_new_object>())
                .def("__len__", &DSocketListPyWrapper::len)
                .def("__iter__", &DSocketListPyWrapper::__iter__, BPy::return_value_policy<BPy::manage_new_object>());
}

DSocketPyWrapper* DSocketListPyWrapper::getitemint(int i)    
{
    if(!alive()) return 0;
    DSocket *s = getWrapped<DSocketList>()->at(i);
    if(!s) return 0;
    if(s->getDir() == IN) {
        if(s->getArray())
            return new DAInSocketPyWrapper((DAInSocket*)s);
        return new DinSocketPyWrapper(s->toIn());
    }
    return new DoutSocketPyWrapper(s->toOut());
}

unsigned int DSocketListPyWrapper::len()    
{
    if(!alive()) return -1;
    return getWrapped<DSocketList>()->len();
}

DSocketPyWrapper* DSocketListPyWrapper::getitemstr(std::string str)    
{
    if(!alive()) return 0;
    for(DSocket *s : *getWrapped<DSocketList>()) {
        if(s->getName() == str) {
            if(s->getDir() == IN){
                if(s->getArray())
                    return new DAInSocketPyWrapper((DAInSocket*)s);
                return new DinSocketPyWrapper(s->toIn());
            }
            return new DoutSocketPyWrapper(s->toOut());
        }
    }
}

std::string DSocketListPyWrapper::__str__()    
{
    if(!alive())return std::string();
    std::string str("[");
    LLsocket *iter = getWrapped<DSocketList>()->getFirst();
    while(iter){
        str += "\""+iter->socket->getName()+"\"";
        if(iter->next) str += " ";
        iter = iter->next;
    }
    str += "]";
    return str;
}

std::string DSocketListPyWrapper::__repr__()    
{
    return "<"+__str__()+">";
}

LLsocketPyWrapper* DSocketListPyWrapper::__iter__()    
{
    if(!alive()) return 0;
    return new LLsocketPyWrapper(getWrapped<DSocketList>()->getFirst());
}

LLsocketPyWrapper::LLsocketPyWrapper(LLsocket* socket)
    : PyWrapper(socket), iterator(new LLsocket)
{
    iterator->next = socket;
}

LLsocketPyWrapper::~LLsocketPyWrapper()
{
}

void LLsocketPyWrapper::wrap()    
{
    BPy::class_<LLsocketPyWrapper>("SocketListIterator", BPy::no_init)
            .def("__iter__", &LLsocketPyWrapper::__iter__, BPy::return_value_policy<BPy::manage_new_object>())
            .def("next", &LLsocketPyWrapper::__next__, BPy::return_value_policy<BPy::manage_new_object>());
}

LLsocketPyWrapper* LLsocketPyWrapper::__iter__()    
{
    return this;
}

DSocketPyWrapper* LLsocketPyWrapper::__next__()    
{
    if(!alive() || !iterator->next) {
        PyErr_SetNone(PyExc_StopIteration);
        BPy::throw_error_already_set();
        return 0;
    }
    iterator = iterator->next;
    DSocket *s = iterator->socket;
    if(s->getDir() == IN) {
        if(s->getArray()) return new DAInSocketPyWrapper((DAInSocket*)s);
        return new DinSocketPyWrapper(s->toIn());
    }
    return new DoutSocketPyWrapper(s->toOut());
}

DSocketPyWrapper::DSocketPyWrapper(DSocket *socket)
    : PyWrapper(socket)
{
}

DSocketPyWrapper::~DSocketPyWrapper()
{
}

void DSocketPyWrapper::wrap()    
{
    BPy::class_<DSocketPyWrapper>("DSocket", BPy::no_init)
        //.def("getName", &DSocketPyWrapper::getName)
        .add_property("name", &DSocketPyWrapper::getName, &DSocketPyWrapper::setName)
        .add_property("type", &DSocketPyWrapper::getType, &DSocketPyWrapper::setType)
        .add_property("node", 
                BPy::make_function(&DSocketPyWrapper::getNode, 
                    BPy::return_value_policy<BPy::manage_new_object>())); 
        //.def("getNode", &DSocketPyWrapper::getNode, BPy::return_value_policy<BPy::manage_new_object>()); 
}

std::string DSocketPyWrapper::getName()    
{
    if(!alive()) return "";
    return getWrapped<DSocket>()->getName();
}

void DSocketPyWrapper::setName(std::string name)    
{
    if(!alive()) return;
    getWrapped<DSocket>()->setName(name);
}

std::string DSocketPyWrapper::getType()
{
    if(!alive())return "";
    auto t = getWrapped<DSocket>()->getType();
    return t.getCustomType().c_str();
}

void DSocketPyWrapper::setType(std::string value)
{
    if(!alive())return;
    getWrapped<DSocket>()->setType(value);
}

DNodePyWrapper* DSocketPyWrapper::getNode()    
{
    if(!alive()) return 0;
    return new DNodePyWrapper(getWrapped<DSocket>()->getNode());
}

DinSocketPyWrapper::DinSocketPyWrapper(DinSocket *socket)
    : DSocketPyWrapper(socket)
{
}

DinSocketPyWrapper::~DinSocketPyWrapper()
{
}

void DinSocketPyWrapper::wrap()    
{
    BPy::class_<DinSocketPyWrapper, BPy::bases<DSocketPyWrapper> >("DinSocket", BPy::no_init)
            .add_property("connected", 
                    BPy::make_function(&DinSocketPyWrapper::getCntd, 
                        BPy::return_value_policy<BPy::manage_new_object>()),
                    &DinSocketPyWrapper::setCntd)
            .add_property("value", 
                    &DinSocketPyWrapper::getProp,
                    &DinSocketPyWrapper::setProp);
}

BPy::object DinSocketPyWrapper::getProp()
{
    if(!alive()) return BPy::object();
    return getWrapped<DinSocket>()->getProperty().toPython();
}

void DinSocketPyWrapper::setProp(BPy::object value)
{
    if(!alive())return;
    auto prop = Property::createPropertyFromPython("", value);
    auto socket = getWrapped<DinSocket>();
    socket->setProperty(prop);
    if(socket->getType() == "VARIABLE")
        socket->setType(prop.getType());
}

DoutSocketPyWrapper* DinSocketPyWrapper::getCntd()    
{
    if(!alive()) return 0;
    auto s = getWrapped<DinSocket>()->getCntdSocket();
    if(!s)return 0;
    return new DoutSocketPyWrapper(s);
}

void DinSocketPyWrapper::setCntd(DoutSocketPyWrapper *socket)    
{
    if(!alive())return;
    getWrapped<DinSocket>()->setCntdSocket(socket->getWrapped<DoutSocket>());  
}

DoutSocketPyWrapper::DoutSocketPyWrapper(DoutSocket *socket)
    : DSocketPyWrapper(socket)
{
}

DoutSocketPyWrapper::~DoutSocketPyWrapper()
{
}

void DoutSocketPyWrapper::wrap()    
{
    BPy::class_<DoutSocketPyWrapper, BPy::bases<DSocketPyWrapper> >("DoutSocket", BPy::no_init);
}

DAInSocketPyWrapper::DAInSocketPyWrapper(DAInSocket *socket)
    : DSocketPyWrapper(socket)
{
}

DAInSocketPyWrapper::~DAInSocketPyWrapper()
{
}

void DAInSocketPyWrapper::wrap()    
{
    BPy::class_<DAInSocketPyWrapper>("DAInSocket", BPy::no_init);
}

void PropertyPyWrapper::wrap()    
{
    BPy::class_<Property>("Property", BPy::no_init)
        .def("__repr__", &PropertyPyWrapper::__repr__)
        .def("__str__", &PropertyPyWrapper::__str__);    
}

std::string PropertyPyWrapper::__str__(BPy::object self)    
{
    Property prop = BPy::extract<Property>(self);
    return BPy::extract<std::string>(BPy::str(prop.toPython()));
}

std::string PropertyPyWrapper::__repr__(BPy::object self)    
{
   return "<"+__str__(self)+">"; 
}
