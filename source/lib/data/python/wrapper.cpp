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
#include "data/nodes/containernode.h"
#include "data/dnspace.h"
#include "pyexposable.h"
#include "pyutils.h"
#include "graphics/windowlist.h"
#include "graphics/viewer_dock_base.h"
#include "graphics/viewer.h"
#include "data/signal.h"
#include "data/properties.h"
#include "data/debuglog.h"
#include "wrapper.h"

using namespace MindTree;
using namespace MindTree::Python;


DNodeListIteratorPyWrapper::DNodeListIteratorPyWrapper(NodeList nodelist)
    : nodelist(nodelist), _index(0)
{
}

DNodeListIteratorPyWrapper::~DNodeListIteratorPyWrapper()
{
}

void DNodeListIteratorPyWrapper::wrap()
{
    BPy::class_<DNodeListIteratorPyWrapper, BPy::bases<PyWrapper>>("NodeIter", BPy::no_init)
            .def("__iter__", &DNodeListIteratorPyWrapper::iter, BPy::return_value_policy<BPy::manage_new_object>())
            .def("__next__", &DNodeListIteratorPyWrapper::next);
}

DNodeListIteratorPyWrapper* DNodeListIteratorPyWrapper::iter()
{
    return new DNodeListIteratorPyWrapper(nodelist);
}

BPy::object DNodeListIteratorPyWrapper::next()
{
    if(_index >= nodelist.size()) {
        PyErr_SetNone(PyExc_StopIteration);
        BPy::throw_error_already_set();
        return BPy::object();
    }
    DNode *node = nodelist[_index];
    BPy::object obj = getPyObject(node);
    ++_index;
    return obj;
}

PythonNodeDecorator::PythonNodeDecorator(BPy::object cls)
    : cls(cls)
{
    setType(BPy::extract<std::string>(cls.attr("type")));
    setLabel(BPy::extract<std::string>(cls.attr("label")));
}

PythonNodeDecorator::~PythonNodeDecorator()
{
}

DNode* PythonNodeDecorator::operator()(bool raw)
{
    GILLocker releaser;
    DNode *node = new DNode();
    node->setType(getType());
    try{
        cls(getPyObject(node), raw);
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

PyWrapper::PyWrapper(const PyWrapper &other)
    : element(other.element)
{
    if(element) element->regWrapper(this);
}

PyWrapper::~PyWrapper()
{
    if(element)element->rmWrapper(this);
}

void PyWrapper::wrap()
{
    BPy::class_<PyWrapper>("PyWrapper", BPy::no_init)
        .add_property("ptr", &PyWrapper::ptr)
        .def("__eq__", &PyWrapper::equal)
        .def("__ne__", &PyWrapper::notequal);
}

int PyWrapper::ptr()
{
    return reinterpret_cast<intptr_t>(getWrapped<PyExposable>());
}

bool PyWrapper::equal(PyWrapper *other)
{
    if (!alive() || !other || !other->alive()) return false;
    return getWrapped<PyExposable>() == other->getWrapped<PyExposable>();
}

bool PyWrapper::notequal(PyWrapper *other)
{
    if (!alive() || !other || !other->alive()) return false;
    return !equal(other);
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
    element = nullptr;
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
   BPy::class_<ProjectPyWrapper, BPy::bases<PyWrapper>>("Project", BPy::no_init)
            .add_property("filename", &ProjectPyWrapper::getFilename,
                    &ProjectPyWrapper::setFilename)
            .def("save", &ProjectPyWrapper::save)
            .add_property("root",
                    BPy::make_function(&ProjectPyWrapper::getRoot,
                        BPy::return_value_policy<BPy::manage_new_object>()));
}

std::string ProjectPyWrapper::getFilename()
{
    if(!alive()) return std::string();
    return getWrapped<Project>()->getFilename();
}

void ProjectPyWrapper::save()
{
    GILReleaser _;
    if(!alive()) return;
    getWrapped<Project>()->save();
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
    BPy::object(DNSpacePyWrapper::*getItemString)(std::string) = &DNSpacePyWrapper::__getitem__;
    BPy::object(DNSpacePyWrapper::*getItemInt)(int) = &DNSpacePyWrapper::__getitem__;
    BPy::class_<DNSpacePyWrapper, BPy::bases<PyWrapper>>("DNSpace", BPy::no_init)
                .def("addNode", &DNSpacePyWrapper::addNode)
                .def("removeNode", &DNSpacePyWrapper::removeNode)
                .def("isContainer", &DNSpacePyWrapper::isContainer)
                .add_property("name", &DNSpacePyWrapper::getName, &DNSpacePyWrapper::setName)
                .def("__len__", &DNSpacePyWrapper::len)
                .def("__str__", &DNSpacePyWrapper::__str__)
                .def("__repr__", &DNSpacePyWrapper::__repr__)
                .def("__iter__", &DNSpacePyWrapper::iter, BPy::return_value_policy<BPy::manage_new_object>())
                .def("__getitem__", getItemString)
                .def("__getitem__", getItemInt);
}

//helper
BPy::object getPyObject(DNSpace *space)
{
    if(!space) return BPy::object();

    if(space->isContainerSpace())
        return BPy::object(new ContainerSpacePyWrapper(space->toContainer()));
    else
        return BPy::object(new DNSpacePyWrapper(space));
}

void DNSpacePyWrapper::addNode(DNodePyWrapper *node)
{
    if(!node || !(alive() && node->alive())) return;
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

int DNSpacePyWrapper::len()
{
    if(!alive()) return 0;
    return getWrapped<DNSpace>()->getNodeCnt();
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

BPy::object DNSpacePyWrapper::__getitem__(std::string name)
{
    if(!alive()) return BPy::object();
    for(DNode *node : getWrapped<DNSpace>()->getNodes()){
        if(node->getNodeName() == name)
            return getPyObject(node);
    }
    PyErr_SetString(PyExc_KeyError, name.c_str());
    PyErr_Print();
    return BPy::object();
}

BPy::object DNSpacePyWrapper::__getitem__(int index)
{
    if(!alive()) return BPy::object();
    if(getWrapped<DNSpace>()->getNodes().size() <= (size_t)index) {
        std::stringstream ss;
        ss << "list index: " << index << " out of range";
        PyErr_SetString(PyExc_IndexError, ss.str().c_str());
        PyErr_Print();
        return BPy::object();
    }
    return getPyObject(getWrapped<DNSpace>()->getNodes().at(index));
}

DNodeListIteratorPyWrapper* DNSpacePyWrapper::iter()
{
    if(!alive()) return nullptr;
    return new DNodeListIteratorPyWrapper(getWrapped<DNSpace>()->getNodes());
}

ContainerSpacePyWrapper::ContainerSpacePyWrapper(ContainerSpace *space)
    : DNSpacePyWrapper(space)
{
}

ContainerSpacePyWrapper::~ContainerSpacePyWrapper()
{
}

void ContainerSpacePyWrapper::wrap()
{
    BPy::class_<ContainerSpacePyWrapper, BPy::bases<DNSpacePyWrapper>>("ContainerSpace", BPy::no_init)
        .add_property("container", BPy::make_function(&ContainerSpacePyWrapper::getContainer,
                                                      BPy::return_value_policy<BPy::manage_new_object>()))
        .add_property("parent", &ContainerSpacePyWrapper::getParent);
}

ContainerNodePyWrapper* ContainerSpacePyWrapper::getContainer()
{
    if(!alive()) return nullptr;
    return new ContainerNodePyWrapper(getWrapped<ContainerSpace>()->getContainer());
}

BPy::object ContainerSpacePyWrapper::getParent()
{
    if(!alive()) return BPy::object();
    auto *space = getWrapped<ContainerSpace>();
    return getPyObject(space->getParent());
}

DNodePyWrapper::DNodePyWrapper(DNode *node)
    : PyWrapper(node)
{
}

DNodePyWrapper::~DNodePyWrapper()
{
}

//helper
BPy::object MindTree::getPyObject(DNode* node)
{
    if(!node) return BPy::object();

    switch(node->getBuildInType()) {
        case DNode::NODE:
            {
                BPy::object obj(new DNodePyWrapper(node));
                return obj;
            }
        case DNode::CONTAINER:
            {
                BPy::object obj(new ContainerNodePyWrapper(node->getDerived<ContainerNode>()));
                return obj;
            }
    }
}

void DNodePyWrapper::wrap()
{
    BPy::class_<DNodePyWrapper, BPy::bases<PyWrapper>>("DNode", BPy::no_init)
                .add_property("name", &DNodePyWrapper::getName, &DNodePyWrapper::setName)
                .add_property("type", &DNodePyWrapper::getType, &DNodePyWrapper::getType)
                .add_property("pos", &DNodePyWrapper::getPos, &DNodePyWrapper::setPos)
                .add_property("insockets", &DNodePyWrapper::in)
                .add_property("space", &DNodePyWrapper::getSpace)
                .def("addInSocket", &DNodePyWrapper::addInSocket, BPy::return_value_policy<BPy::manage_new_object>())
                .def("addOutSocket", &DNodePyWrapper::addOutSocket, BPy::return_value_policy<BPy::manage_new_object>())
                .add_property("selected", &DNodePyWrapper::getSelected, &DNodePyWrapper::setSelected)
                .def("setDynamicInSockets", &DNodePyWrapper::setDynamicInSockets)
                .add_property("outsockets", &DNodePyWrapper::out);
}

bool DNodePyWrapper::getSelected()
{
    if(!alive()) return false;
    return getWrapped<DNode>()->getSelected();
}

void DNodePyWrapper::setSelected(bool value)
{
    GILReleaser releaser;
    if(!alive())return;
    getWrapped<DNode>()->setSelected(value);
}

void DNodePyWrapper::setDynamicInSockets()
{
    if(!alive()) return;
    getWrapped<DNode>()->setDynamicSocketsNode(DSocket::IN);
}

DinSocketPyWrapper* DNodePyWrapper::addInSocket(std::string name, std::string type)
{
    if(!alive()) return 0;
    return new DinSocketPyWrapper(new DinSocket(name, SocketType(type), getWrapped<DNode>()));
}

DoutSocketPyWrapper* DNodePyWrapper::addOutSocket(std::string name, std::string type)
{
    if(!alive()) return 0;
    return new DoutSocketPyWrapper(new DoutSocket(name, SocketType(type), getWrapped<DNode>()));
}

BPy::object DNodePyWrapper::getSpace()
{
    if(!alive())return BPy::object();
    auto *space = getWrapped<DNode>()->getSpace();
    return getPyObject(space);
}

void DNodePyWrapper::setName(std::string name)
{
    GILReleaser releaser;
    if(!alive())return;
    getWrapped<DNode>()->setName(name);
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
    GILReleaser releaser;
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
    GILReleaser releaser;
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

BPy::list DNodePyWrapper::in()
{
    if(!alive()) return BPy::list();
    BPy::list l;
    for(DinSocket *socket : getWrapped<DNode>()->getInSockets()) {
        l.append(new DinSocketPyWrapper(socket));
    }
    return l;
}

BPy::list DNodePyWrapper::out()
{
    if(!alive())return BPy::list();
    BPy::list l;
    for(DoutSocket *socket : getWrapped<DNode>()->getOutSockets()) {
        l.append(new DoutSocketPyWrapper(socket));
    }
    return l;
}

ContainerNodePyWrapper::ContainerNodePyWrapper(ContainerNode *node)
    : DNodePyWrapper(node)
{
}

ContainerNodePyWrapper::~ContainerNodePyWrapper()
{
}

void ContainerNodePyWrapper::wrap()
{
    BPy::class_<ContainerNodePyWrapper, BPy::bases<DNodePyWrapper>>("ContainerNode", BPy::no_init)
        .add_property("graph",
                      BPy::make_function(&ContainerNodePyWrapper::getGraph,
                                         BPy::return_value_policy<BPy::manage_new_object>()));
}

ContainerSpacePyWrapper* ContainerNodePyWrapper::getGraph()
{
    if(!alive()) return nullptr;
    return new ContainerSpacePyWrapper(getWrapped<ContainerNode>()->getContainerData());
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
    BPy::class_<DSocketPyWrapper, BPy::bases<PyWrapper>>("DSocket", BPy::no_init)
        //.def("getName", &DSocketPyWrapper::getName)
        .add_property("name", &DSocketPyWrapper::getName, &DSocketPyWrapper::setName)
        .add_property("type", &DSocketPyWrapper::getType, &DSocketPyWrapper::setType)
        .add_property("node", &DSocketPyWrapper::getNode);
        //.def("getNode", &DSocketPyWrapper::getNode, BPy::return_value_policy<BPy::manage_new_object>());
}

std::string DSocketPyWrapper::getName()
{
    if(!alive()) return "";
    return getWrapped<DSocket>()->getName();
}

void DSocketPyWrapper::setName(std::string name)
{
    GILReleaser releaser;
    if(!alive()) return;
    getWrapped<DSocket>()->setName(name);
}

std::string DSocketPyWrapper::getType()
{
    if(!alive())return "";
    auto t = getWrapped<DSocket>()->getType();
    return t.toStr();
}

void DSocketPyWrapper::setType(std::string value)
{
    GILReleaser releaser;
    if(!alive())return;
    getWrapped<DSocket>()->setType(value);
}

BPy::object DSocketPyWrapper::getNode()
{
    if(!alive()) return BPy::object();
    return getPyObject(getWrapped<DSocket>()->getNode());
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
    auto prop = Property::createPropertyFromPython(value);
    {
        GILReleaser releaser;
        auto socket = getWrapped<DinSocket>();
        socket->setProperty(prop);
        socket->setType(prop.getType());
    }
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
    GILReleaser releaser;
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
    BPy::class_<DoutSocketPyWrapper, BPy::bases<DSocketPyWrapper> >("DoutSocket", BPy::no_init)
        .add_property("cntdSockets", &DoutSocketPyWrapper::getCntdSockets);
}

BPy::object DoutSocketPyWrapper::getCntdSockets()
{
    if(!alive()) return BPy::object();

    BPy::list l;

    for(DinSocket *socket : getWrapped<DoutSocket>()->getCntdSockets()) {
        l.append(new DinSocketPyWrapper(socket));
    }

    return l;
}

void PropertyPyWrapper::wrap()
{
    BPy::class_<Property, BPy::bases<PyWrapper>>("Property", BPy::no_init)
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
