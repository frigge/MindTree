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

#ifndef WRAPPER_FA9WP8AJ

#define WRAPPER_FA9WP8AJ

#include "boost/python.hpp"
#include "iostream"

#include "glm/glm.hpp"
#include "memory"
#include "data/nodes/node_db.h"
#include "data/windowfactory.h"

namespace BPy = boost::python;
namespace MindTree
{

class ContainerSpace;
class ContainerNode;
class DNSpace;
class DNode;
typedef std::shared_ptr<DNode> NodePtr;
typedef std::vector<NodePtr> NodeList;

namespace utils
{
BPy::object getPyObject(NodePtr node);
BPy::object getPyObject(DNode *node);
BPy::object getPyObject(DNSpace *space);

NodePtr getNodePtr(DNode* node);
}

template<class T>
struct PyConverter{
    static BPy::object pywrap(T data){ return BPy::object(data); }
    typedef T t;
};

template<typename T>
struct PyConverter<std::vector<T>>{
    static BPy::object pywrap(std::vector<T> data)
    {
        BPy::list l;
        for (auto d : data)
            l.append(d);

        return l;
    }
    typedef std::vector<T> t;
};

#ifndef PYWRAP_map_types
#define PYWRAP_map_types(oldtype, newtype)\
template<>\
struct PyConverter<oldtype>{\
    static BPy::object pywrap(oldtype data){return BPy::object(data);}\
    typedef newtype t;\
};
#endif

#ifndef PYWRAPPERFUNC
#define PYWRAPPERFUNC(type) \
template<> \
struct PyConverter<type*> { \
    static BPy::object pywrap(PyExposable *data) { return BPy::object(new type##PyWrapper((type *)data));} \
    typedef type##PyWrapper t; \
};
#endif

#ifndef PYWRAPPERCLASS
#define PYWRAPPERCLASS(type) \
class type##PyWrapper : public PyWrapper
#endif

#ifndef PYWRAPHEAD
#define PYWRAPHEAD(type) \
public: \
    type##PyWrapper(type *data); \
    virtual ~type##PyWrapper(); \
    static void wrap();
#endif

template<>
struct PyConverter<glm::ivec2>
{
    static BPy::object pywrap(glm::ivec2 data)
    {
        return BPy::make_tuple(data[0], data[1]);
    }
};

template<>
struct PyConverter<glm::vec3>
{
    static BPy::object pywrap(glm::vec3 data)
    {
        return BPy::make_tuple(data[0], data[1], data[2]);
    }
};

template<>
struct PyConverter<glm::vec4>
{
    static BPy::object pywrap(glm::vec4 data)
    {
        return BPy::make_tuple(data[0], data[1], data[2], data[3]);
    }
};

class DNodePyWrapper;
class DNodeListIteratorPyWrapper
{
public:
    DNodeListIteratorPyWrapper(NodeList nodelist);
    virtual ~DNodeListIteratorPyWrapper();
    static void wrap();
    DNodeListIteratorPyWrapper* iter();
    BPy::object next();

private:
    size_t _index;
    NodeList nodelist;
};

class PythonNodeDecorator : public MindTree::AbstractNodeDecorator
{
public:
    PythonNodeDecorator(BPy::object cls);
    virtual ~PythonNodeDecorator();
    NodePtr createNode(bool);

private:
    BPy::object cls;
    std::string base_;
};

class PyExposable;
namespace Signal {class CallbackHandler;}
class DoutSocketPyWrapper;
class PyWrapper
{
public:
    PyWrapper(PyExposable *exp);
    PyWrapper(const PyWrapper &other);
    virtual ~PyWrapper();

    static void wrap();

    int ptr();
    bool equal(PyWrapper *other);
    bool notequal(PyWrapper *other);
    void elementDestroyed();
    template<class T>
    T* getWrapped() const
    {
        return dynamic_cast<T*>(element);
    }

    bool alive() const;
#ifdef QT_DEBUG
    static QString getStylePath();
    static void setStylePath(QString path);
#endif

private:
    mutable PyExposable *element;
#ifdef QT_DEBUG
    static QString stylePath;
#endif
};

class Project;
class DSocket;
class DinSocket;
class DoutSocket;

class DNSpacePyWrapper;

PYWRAPPERCLASS(Project)
{
    PYWRAPHEAD(Project)
    std::string getFilename();
    void setFilename(std::string name);
    void save();
    DNSpacePyWrapper* getRoot();
};
PYWRAPPERFUNC(Project)

class DNodePyWrapper;
PYWRAPPERCLASS(DNSpace)
{
public:
    PYWRAPHEAD(DNSpace);
    std::string getName();
    void addNode(DNodePyWrapper *node);
    void removeNode(DNodePyWrapper *node);
    bool isContainer();
    void setName(std::string name);
    int len();
    std::string __str__();
    std::string __repr__();
    BPy::object __getitem__(std::string name);
    BPy::object __getitem__(int index);
    DNodeListIteratorPyWrapper* iter();
};
PYWRAPPERFUNC(DNSpace)

class ContainerNodePyWrapper;
class ContainerSpacePyWrapper: public DNSpacePyWrapper
{
public:
    PYWRAPHEAD(ContainerSpace);

    ContainerNodePyWrapper* getContainer();
    BPy::object getParent();
};
PYWRAPPERFUNC(ContainerSpace)

class DinSocketPyWrapper;
class DoutSocketPyWrapper;
class DNodePyWrapper : public PyWrapper
{
public:
    DNodePyWrapper(NodePtr node);
    DNodePyWrapper(DNode *node);

    virtual ~DNodePyWrapper();

    static void init(BPy::object self);
    static void wrap();
    bool getSelected();
    void setSelected(bool sel);
    std::string getName();
    void setType(std::string value);
    std::string getType();
    BPy::list getNodeTypes();
    void setName(std::string name);
    BPy::tuple getPos();
    void setPos(BPy::tuple pos);
    BPy::list in();
    BPy::list out();
    BPy::object getProperty(std::string name);
    DinSocketPyWrapper* addInSocket(std::string name, std::string type);
    DoutSocketPyWrapper* addOutSocket(std::string name, std::string type);
    BPy::object getSpace();

    NodePtr getNodePtr() const;

private:
    NodePtr _node;
};

template<>
struct PyConverter<DNode*> {
    static BPy::object pywrap(DNode *data)
    {
        return utils::getPyObject(data);
    }
};

template<>
struct PyConverter<NodePtr> {
    static BPy::object pywrap(NodePtr data) { return utils::getPyObject(data); }
};

class ContainerSpacePyWrapper;
class ContainerNodePyWrapper: public DNodePyWrapper
{
public:
    ContainerNodePyWrapper(std::shared_ptr<ContainerNode> node);
    ContainerNodePyWrapper(ContainerNode *node);

    virtual ~ContainerNodePyWrapper();

    static void wrap();

    ContainerSpacePyWrapper* getGraph();

};

template<>
struct PyConverter<std::shared_ptr<ContainerNode>>
{
    static BPy::object pywrap(std::shared_ptr<ContainerNode> data)
    {
        return BPy::object(new ContainerNodePyWrapper(data));
    }

    typedef ContainerNodePyWrapper t;
};

class DSocketPyWrapper : public PyWrapper
{
public:
    DSocketPyWrapper(DSocket *socket);
    virtual ~DSocketPyWrapper();
    static void wrap();
    BPy::object getNode();
    std::string getName();
    void setName(std::string name);
    void setType(std::string value);
    std::string getType();
    std::string getDir() const;
};
PYWRAPPERFUNC(DSocket)

class DoutSocketPyWrapper;
class DinSocketPyWrapper : public DSocketPyWrapper
{
public:
    DinSocketPyWrapper(DinSocket *socket);
    virtual ~DinSocketPyWrapper();
    DoutSocketPyWrapper* getCntd();
    void setCntd(DoutSocketPyWrapper *socket);
    BPy::object getProp();
    void setProp(BPy::object value);
    static void wrap();
    BPy::list getChildNodes() const;
};
PYWRAPPERFUNC(DinSocket)

class DoutSocketPyWrapper : public DSocketPyWrapper
{
public:
    DoutSocketPyWrapper(DoutSocket *socket);
    virtual ~DoutSocketPyWrapper();
    static void wrap();

    BPy::object getCntdSockets();
};
PYWRAPPERFUNC(DoutSocket)

class PropertyPyWrapper
{
public:
    static void wrap();

    static std::string __str__(BPy::object self);
    static std::string __repr__(BPy::object self);
};
} /* MindTree */
#endif /* end of include guard: WRAPPER_FA9WP8AJ */
