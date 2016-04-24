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

#include "data/signal.h"
#include "data/mtobject.h"

#include "functional"
#include "memory"

namespace MindTree {
class DNode;

typedef std::shared_ptr<DNode> NodePtr;

class LoadNodeIDMapper
{
public:
    static unsigned short getID(NodePtr node);
    static void setID(NodePtr node, unsigned short ID);
    static NodePtr getNode(unsigned short ID);
    static void clear();

private:
    static std::unordered_map<unsigned short, NodePtr>loadIDMapper;
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

typedef std::vector<NodePtr> NodeList;
typedef std::vector<const DNode*> ConstNodeList;


class DNode : public Object, public PyExposable
{
public:
    enum BuildInType {
        NODE = 1,
        CONTAINER = 2,
        SOCKETNODE = 3
    };

    DNode(std::string name="");
    explicit DNode(const DNode& node);

    static NodeList copy(NodeList nodes);
    virtual ~DNode();

    DNode::BuildInType getBuildInType() const;
    bool getSelected();
    void setSelected(bool value);

    const NodeType& getType()const;
    void setType(NodeType value);
    virtual NodePtr clone();

    const Vec2i& getPos()const;
    void setPos(Vec2i value);

    template<class C>
    const C* getDerivedConst() const;

    template<class C>
    C* getDerived();

    virtual std::vector<DNode*> getAllInNodes();
    virtual ConstNodeList getAllInNodesConst() const;

    virtual void setName(std::string name);
    std::string getNodeName() const;
    virtual void addSocket(DSocket *socket);
    std::vector<std::string> getSocketNames();
    void setSocketIDName(DSocket *socket);
    DSocket* getSocketByIDName(std::string idname);
    void removeSocket(DSocket *socket);
    void clearSocketLinks();
    bool isContainer() const;

    unsigned short getID() const;
    void setID(unsigned short value);
    void setOutSockets(DoutSocketList value);
    DoutSocketList getOutSockets() const;
    DinSocketList getInSockets() const;
    void setInSockets(DinSocketList value);
    DNSpace* getSpace() const;
    virtual void setSpace(DNSpace* value);

    static NodePtr newNode(std::string name, NodeType t, int insize, int outsize);
    static bool isInput(const DNode *node);
    
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;

    DNode *createFuncNode(std::string filepath);
    static DNode *dropNode(std::string filepath);

protected:
    inline void setBuildInType(BuildInType t)
    {
        _buildInType = t;
    }

private:
    BuildInType _buildInType;
    static std::vector<std::function<NodePtr()>> newNodeDecorator;

    bool selected;
    DNSpace *space;
    unsigned short ID;
    static unsigned short count;
    std::string nodeName;
    mutable DSocketList outSockets;
    mutable DSocketList inSockets;
    NodeType type;
    Vec2i pos;

    std::unique_ptr<Signal::LiveTimeTracker> _signalLiveTime;
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
