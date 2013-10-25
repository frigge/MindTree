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

#include "QPointF"
#include "QString"
#include "boost/python.hpp"
#include "iostream"

#include "data/nodes/node_db.h"
#include "data/windowfactory.h"

namespace BPy = boost::python;
namespace MindTree
{
    
class DNode;
class LLsocket;
class DSocketList;

template<class T>
struct PyConverter{
    static BPy::object pywrap(T data){ return BPy::object(data); }
    typedef T t;
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

class DNodePyWrapper;
class DNodeListIteratorPyWrapper
{
public:
    DNodeListIteratorPyWrapper(QList<DNode*> nodelist);
    virtual ~DNodeListIteratorPyWrapper();
    static void wrap();
    DNodeListIteratorPyWrapper* iter();
    DNodePyWrapper* next();

private:
    QList<DNode*>::iterator iterator;
    QList<DNode*> nodelist;
};

class PythonNodeFactory : public MindTree::AbstractNodeFactory
{
public:
    PythonNodeFactory(BPy::object cls);
    virtual ~PythonNodeFactory();
    DNode* operator()();

private:
    BPy::object cls;
};

class PyExposable;
namespace Signal {class CallbackHandler;}
class DoutSocketPyWrapper;
class PyWrapper
{
public:
    PyWrapper(PyExposable *exp);
    virtual ~PyWrapper();
    static void regNode(QString group, QString name, PyObject *nodeClass);
    static DNodePyWrapper* createNode(std::string name);
    static BPy::list getRegisteredNodes();
    static Signal::CallbackHandler attachToSignal(QString id, BPy::object fn);
    static QList<QString> getNodeTypes();
    static std::string __str__StringVector(std::vector<std::string> &self);
    static std::string __repr__StringVector(std::vector<std::string> &self);
    static std::vector<std::string> getSocketTypes();
    void elementDestroyed();
    template<class T>
    T* getWrapped() const
    {
        return static_cast<T*>(element);
    }

    bool alive();
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
class DNSpace;
class DNode;
class DSocket;
class DinSocket;
class DoutSocket;
class DAInSocket;

class DNSpacePyWrapper;

PYWRAPPERCLASS(Project)
{
    PYWRAPHEAD(Project)
    QString getFilename();
    void setFilename(QString name);
    DNSpacePyWrapper* getRoot();
};
PYWRAPPERFUNC(Project)

//class ProjectPyWrapper : public PyWrapper
//{
//public:
//    ProjectPyWrapper(Project *project);
//    virtual ~ProjectPyWrapper();
//    static void wrap();
//    QString getFilename();
//    void setFilename(QString name);
//    DNSpacePyWrapper* getRoot();
//};

class DNodePyWrapper;
PYWRAPPERCLASS(DNSpace)
{
public:
    PYWRAPHEAD(DNSpace);
    QString getName();
    void addNode(DNodePyWrapper *node);
    void removeNode(DNodePyWrapper *node);
    bool isContainer();
    void setName(QString name);
    QString __str__();
    QString __repr__();
    DNodePyWrapper* __getitem__(QString name);
    DNodePyWrapper* __getitem__(int index);
    DNodeListIteratorPyWrapper* iter();
};
PYWRAPPERFUNC(DNSpace)

class DSocketListPyWrapper;
class DNodePyWrapper : public PyWrapper
{
public:
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
    QPointF getPos();
    void setPos(BPy::tuple pos);
    DSocketListPyWrapper* in();
    DSocketListPyWrapper* out();
    BPy::object getProperty(std::string name);
    void addInSocket(std::string name, std::string type);
    void addOutSocket(std::string name, std::string type);
    DNSpacePyWrapper* getSpace();
    static void setattr(BPy::object self, BPy::object name, BPy::object value);
    static BPy::object getattr(BPy::object self, std::string key);
    static BPy::list dir(BPy::object self);
};
PYWRAPPERFUNC(DNode)

class DSocketPyWrapper;
class LLsocketPyWrapper : public PyWrapper
{
public:
    LLsocketPyWrapper(LLsocket *socket);
    virtual ~LLsocketPyWrapper();
    static void wrap();
    LLsocketPyWrapper* __iter__();
    DSocketPyWrapper* __next__();

private:
    LLsocket *iterator;
};
PYWRAPPERFUNC(LLsocket)

class DSocketListPyWrapper : public PyWrapper
{
public:
    DSocketListPyWrapper(DSocketList *list);
    virtual ~DSocketListPyWrapper();
    static void wrap();
    std::string __str__();
    std::string __repr__();
    LLsocketPyWrapper* __iter__();
    DSocketPyWrapper* getitemint(int i);
    DSocketPyWrapper* getitemstr(QString str);
    unsigned int len();

private:
};
PYWRAPPERFUNC(DSocketList)

class DSocketPyWrapper : public PyWrapper
{
public:
    DSocketPyWrapper(DSocket *socket);
    virtual ~DSocketPyWrapper();
    static void wrap();
    DNodePyWrapper* getNode();
    std::string getName();
    void setName(std::string name);
    void setType(QString value);
    QString getType();

private:
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
};
PYWRAPPERFUNC(DinSocket)

class DoutSocketPyWrapper : public DSocketPyWrapper
{
public:
    DoutSocketPyWrapper(DoutSocket *socket);
    virtual ~DoutSocketPyWrapper();
    static void wrap();
};
PYWRAPPERFUNC(DoutSocket)

class DAInSocketPyWrapper : public DSocketPyWrapper
{
public:
    DAInSocketPyWrapper(DAInSocket *socket);
    virtual ~DAInSocketPyWrapper();
    static void wrap();
};
PYWRAPPERFUNC(DAInSocket)

class PropertyPyWrapper
{
public:
    static void wrap();

    static std::string __str__(BPy::object self);
    static std::string __repr__(BPy::object self);
};
} /* MindTree */
#endif /* end of include guard: WRAPPER_FA9WP8AJ */
