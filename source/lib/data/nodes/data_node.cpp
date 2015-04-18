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


#include "data/dnspace.h"
#include "data/project.h"
#include "iostream"
#include "data/properties.h"
#include "data/project.h"


#include "data_node.h"

using namespace MindTree;

unsigned short DNode::count = 1;
std::unordered_map<unsigned short, NodePtr>LoadNodeIDMapper::loadIDMapper;
std::unordered_map<DNode*, DNode*> CopyNodeMapper::nodeMap;
std::vector<std::function<NodePtr()>> DNode::newNodeDecorator;

unsigned short LoadNodeIDMapper::getID(NodePtr node)
{
    for (auto p : loadIDMapper)
        if (p.second == node) return p.first;

    return -1;
}

void LoadNodeIDMapper::setID(NodePtr node, unsigned short ID)
{
    loadIDMapper.insert({ID, node});
}

NodePtr LoadNodeIDMapper::getNode(unsigned short id)
{
    return loadIDMapper[id];
}

void LoadNodeIDMapper::clear()    
{
    loadIDMapper.clear();
}

void CopyNodeMapper::setNodePair(DNode *original, DNode *copy)    
{
   nodeMap.insert({original, copy}); 
}

DNode * CopyNodeMapper::getCopy(DNode *original)    
{
    if(nodeMap.find(original) == nodeMap.end()) return 0;
    return nodeMap[original];
}

DNode::DNode(std::string name)
        : selected(false),
          space(nullptr),
          varsocket(nullptr),
          lastsocket(nullptr),
          varcnt(0),
          ID(count++),
          nodeName(name),
          _signalLiveTime(new Signal::LiveTimeTracker(this)),
          _buildInType(NODE)
{
}

DNode::DNode(const DNode& node)
: selected(false),
    space(nullptr),
    varcnt(0),
    ID(count++),
    nodeName(node.nodeName),
    type(node.getType()),
    _signalLiveTime(new Signal::LiveTimeTracker(this)),
    _buildInType(node._buildInType)
{
    for(DinSocket *socket : node.getInSockets())
        new DinSocket(*socket, this);
    for(DoutSocket *socket : node.getOutSockets())
        new DoutSocket(*socket, this);
    varsocket = const_cast<DSocket*>(CopySocketMapper::getCopy(node.getVarSocket()));
    lastsocket = const_cast<DSocket*>(CopySocketMapper::getCopy(node.getLastSocket()));

    CopyNodeMapper::setNodePair(const_cast<DNode*>(&node), this);
    setPos(node.getPos());
}

DNode::~DNode()
{
    MT_CUSTOM_SIGNAL_EMITTER("nodeDeleted", this);
    for (auto *in : getInSockets())
        in->clearLink();

    for (auto *out : getOutSockets())
        for (auto *in : out->getCntdSockets())
            in->clearLink();

    for(DinSocket *socket : getInSockets())
        delete socket;
    for(DoutSocket *socket : getOutSockets())
        delete socket;
}

DNode::BuildInType DNode::getBuildInType() const
{
    return _buildInType;
}

bool DNode::getSelected()
{
    return selected;
}

void DNode::setSelected(bool value)
{
    MT_CUSTOM_SIGNAL_EMITTER("selectionChanged", this);
    selected = value;
}

const Vec2i& DNode::getPos()const
{
    return pos;
}

void DNode::setPos(Vec2i value)
{
    pos = value;
    MT_CUSTOM_BOUND_SIGNAL_EMITTER(_signalLiveTime.get(), "nodePositionChanged");
}

DSocket* DNode::getSocketByIDName(std::string idname)    
{
    for(auto *socket : getInSockets())
        if(socket->getIDName() == idname)
            return socket;
    for(auto *socket : getOutSockets())
        if(socket->getIDName() == idname)
            return socket;
    return 0;
}

const NodeType& DNode::getType()const
{
    return type;
}

void DNode::setType(NodeType value)
{
    type = value;
}

NodePtr DNode::clone()
{
    return std::make_shared<DNode>(*this);
}

NodeList DNode::copy(NodeList nodes)    
{
   NodeList nodeCopies;
   for(auto node : nodes)
        nodeCopies.push_back(node->clone());
   CopySocketMapper::remap();
   return nodeCopies;
}

std::vector<DNode*> DNode::getAllInNodes()
{
    std::vector<DNode*> nodes;
    nodes.push_back(this);
    for (auto socket : inSockets) {
        if(socket->toIn()->getCntdSocket()) {
            DNode *nextNode = socket->toIn()->getCntdSocket()->getNode();
            std::vector<DNode*> morenodes = nextNode->getAllInNodes();
            nodes.insert(nodes.end(), morenodes.begin(), morenodes.end());
        }
    }
    return nodes;
}

ConstNodeList DNode::getAllInNodesConst() const
{
    ConstNodeList nodes;
    nodes.push_back(this);
    for (auto socket : inSockets) {
        if(socket->toIn()->getCntdSocket()) {
            const DNode *nextNode = socket->toIn()->getCntdSocket()->getNode();
            ConstNodeList morenodes = nextNode->getAllInNodesConst();
            nodes.insert(nodes.end(), morenodes.begin(), morenodes.end());
        }
    }
    return nodes;
}

bool DNode::isContainer() const
{
    return true;
}

NodePtr DNode::newNode(std::string name, NodeType t, int insize, int outsize)
{
    NodePtr node = newNodeDecorator[t.id()](); 
    node->setName(name);
    return node;
}

bool DNode::operator==(const DNode &node)const
{
    if(type != node.type)
        return false;

    if(nodeName != node.nodeName)
        return false;

    if(getInSockets().size() != node.getInSockets().size()
            ||getOutSockets().size() != node.getOutSockets().size())
        return false;

    for(int i=0; i<getInSockets().size(); i++)
        if(*getInSockets().at(i) != *node.getInSockets().at(i))
            return false;

    for(int i=0; i<getOutSockets().size(); i++)
        if(*getOutSockets().at(i) != *node.getOutSockets().at(i))
            return false;
    return true;
}

bool DNode::operator!=(const DNode &node)const
{
    return (!(*this == node));
}

void DNode::setName(std::string name)
{
    nodeName = name;
}

void DNode::addSocket(DSocket *socket)
{
    setSocketIDName(socket);
    if(socket->getDir()== DSocket::IN) inSockets.push_back(socket);
    else outSockets.push_back(socket);
}

void DNode::setSocketIDName(DSocket *socket)    
{
    std::vector<std::string> socketNames = getSocketNames();
    auto b = socketNames.begin();
    auto e = socketNames.end();

    std::string oldidname = socket->getIDName();
    std::string idname = oldidname;
    uint i = 1;
    while(std::find(b, e, idname) != e)
    {
        idname = socket->getName() + std::to_string(i);
        i++;
    }
    if (idname != oldidname) socket->setIDName(idname);
}

std::vector<std::string> DNode::getSocketNames()    
{
    std::vector<std::string> names;

    for(auto *socket : getInSockets())
        names.push_back(socket->getIDName());
    for(auto *socket : getOutSockets())
        names.push_back(socket->getIDName());

    return names;
}

void DNode::removeSocket(DSocket *socket)
{
    if(!socket)return;
    if(socket->getDir() == DSocket::IN) {
        auto it = std::find(inSockets.begin(), inSockets.end(), socket);
        inSockets.erase(it);
        delete socket;
    }
    else {
        auto it = std::find(outSockets.begin(), outSockets.end(), socket);
        outSockets.erase(it);
        delete socket;
    }
    if (varsocket == socket)
        varsocket = nullptr;

    if (socket == lastsocket)
        lastsocket = nullptr;
}

void DNode::decVarSocket(DSocket *socket)
{
    removeSocket(socket);
    varcnt--;
}

void DNode::incVarSocket()
{
    lastsocket = varsocket;
    if(lastsocket->getDir() == DSocket::IN)
        varsocket = new DinSocket("+", "VARIABLE", this);
    else
        varsocket = new DoutSocket("+", "VARIABLE", this);
    varsocket->setVariable(true);
    varsocket->listenToLinked();
    varcnt++;
}

void DNode::clearSocketLinks()
{
    for(DinSocket *socket : getInSockets())
       socket->clearLink();
}

std::string DNode::getNodeName() const
{
    return nodeName;
}

unsigned short DNode::getID() const
{
    return ID;
}

void DNode::setID(unsigned short value)
{
    ID = value;
}

DoutSocketList DNode::getOutSockets() const
{
    DoutSocketList out;
    for(DSocket *socket : outSockets)
        out.push_back(socket->toOut());
    return out;
}

DSocketList *DNode::getOutSocketLlist() const
{
    return &outSockets;
}

void DNode::setOutSockets(DoutSocketList value)
{
}

DinSocketList DNode::getInSockets() const
{
    DinSocketList in;
    for(DSocket *socket : inSockets)
        in.push_back(socket->toIn());
    return in;
}

DSocketList* DNode::getInSocketLlist()    const
{
    return &inSockets;
}

void DNode::setInSockets(DinSocketList value)
{
}

DSocket* DNode::getVarSocket() const
{
    return varsocket;
}

void DNode::setVarSocket(const DSocket* value)
{
    varsocket = const_cast<DSocket*>(value);
}

DSocket* DNode::getLastSocket() const
{
    return lastsocket;
}

void DNode::setLastSocket(const DSocket* value)
{
    lastsocket = const_cast<DSocket*>(value);
}

int DNode::getVarcnt() const
{
    return varcnt;
}

void DNode::setVarcnt(int value)
{
    varcnt = value;
}

DNSpace* DNode::getSpace() const
{
    return space;
}

void DNode::setSpace(DNSpace* value)
{
    space = value;
}

void DNode::setDynamicSocketsNode(DSocket::SocketDir dir)
{
    if(dir == DSocket::IN)
        varsocket = new DinSocket("+", "VARIABLE", this);
    else
        varsocket = new DoutSocket("+", "VARIABLE", this);
    varsocket->setVariable(true);
    varsocket->listenToLinked();
}

