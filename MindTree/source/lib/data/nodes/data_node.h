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

#ifndef DATA_NODE_H
#define DATA_NODE_H

#include "data/nodes/data_node_socket.h"
#include "data/python/pyexposable.h"
#include "data/nodes/nodetype.h"
#include "data/datatypes.h"

#include "data/signal.h"
#include "data/mtobject.h"

#include "functional"
#include "memory"

namespace MindTree {
class DNode;

typedef std::shared_ptr<DNode> DNode_ptr;

class LoadNodeIDMapper
{
public:
    static unsigned short getID(DNode_ptr node);
    static void setID(DNode_ptr node, unsigned short ID);
    static DNode_ptr getNode(unsigned short ID);
    static void clear();

private:
    static std::unordered_map<unsigned short, DNode_ptr>loadIDMapper;
};

class CopyNodeMapper
{
public:
    static void setNodePair(DNode *original, DNode *copy);
    static DNode * getCopy(DNode *original);

private:
    static std::unordered_map<DNode*, DNode*> nodeMap;
};

class DNSpace;
class ContainerNode;

typedef std::vector<DNode*> NodeList;
typedef std::vector<const DNode*> ConstNodeList;

namespace IO {
    void write(std::ostream&, const DNode*);
}

class DNode : public Object, public PyExposable
{
public:
    DNode(std::string name="");
    DNode(const DNode& node);

    static NodeList copy(NodeList nodes);
    virtual ~DNode();

    bool getSelected();
    void setSelected(bool value);

    NodeType getType()const;
    void setType(NodeType value);
    virtual DNode* clone();

    Vec2i getPos()const;
    void setPos(Vec2i value);

    template<class C>
    const C* getDerivedConst() const;

    template<class C>
    C* getDerived();

    virtual NodeList getAllInNodes();
    virtual ConstNodeList getAllInNodesConst() const;

    virtual void setNodeName(std::string name);
    std::string getNodeName() const;
    void setNodeType(NodeType t);
    virtual void addSocket(DSocket *socket);
    std::vector<std::string> getSocketNames();
    void setSocketIDName(DSocket *socket);
    DSocket* getSocketByIDName(std::string idname);
    void removeSocket(DSocket *socket);
    virtual void dec_var_socket(DSocket *socket);
    virtual void inc_var_socket();
    void setDynamicSocketsNode(DSocket::SocketDir dir);
    void clearSocketLinks();
    bool isContainer() const;

    unsigned short getID() const;
    void setID(unsigned short value);
    void setOutSockets(DoutSocketList value);
    DoutSocketList getOutSockets() const;
    DinSocketList getInSockets() const;
    DSocketList* getInSocketLlist() const;
    DSocketList *getOutSocketLlist() const;
    void setInSockets(DinSocketList value);
    DSocket* getVarSocket() const;
    void setVarSocket(const DSocket* value);
    DSocket* getLastSocket() const;
    void setLastSocket(const DSocket* value);
    int getVarcnt() const;
    void setVarcnt(int value);
    DNSpace* getSpace() const;
    virtual void setSpace(DNSpace* value);

    static DNode_ptr newNode(std::string name, NodeType t, int insize, int outsize);

    static bool isInput(const DNode *node);
    static bool isConditionNode(const DNode *node);
    bool isValueNode()const;
    
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;

    DNode *createFuncNode(std::string filepath);
    static DNode *dropNode(std::string filepath);

private:
    static std::vector<std::function<DNode_ptr()>> newNodeDecorator;

    friend void MindTree::IO::write(std::ostream&, const DNode*);

    bool selected;
    DNSpace *space;
    DSocket *varsocket;
    DSocket *lastsocket;
    int varcnt;
    unsigned short ID;
    static unsigned short count;
    std::string nodeName;
    mutable DSocketList outSockets;
    mutable DSocketList inSockets;
    NodeType type;
    Vec2i pos;

    Signal::LiveTimeTracker *_signalLiveTime;
};

template<class C>
const C* DNode::getDerivedConst() const
{
    return static_cast<const C*>(this); 
}

template<class C>
C* DNode::getDerived()
{
    return static_cast<C*>(this); 
}

}

#endif // DATA_NODE_H
