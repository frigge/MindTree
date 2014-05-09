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
std::unordered_map<unsigned short, DNode_ptr>LoadNodeIDMapper::loadIDMapper;
std::unordered_map<DNode*, DNode*> CopyNodeMapper::nodeMap;
std::vector<std::function<DNode_ptr()>> DNode::newNodeFactory;

unsigned short LoadNodeIDMapper::getID(DNode_ptr node)
{
    for (auto p : loadIDMapper)
        if (p.second == node) return p.first;

    return -1;
}

void LoadNodeIDMapper::setID(DNode_ptr node, unsigned short ID)
{
    loadIDMapper.insert({ID, node});
}

DNode_ptr LoadNodeIDMapper::getNode(unsigned short id)
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

void IO::write(std::ostream &stream, const DNode *node)
{
    stream.write(node->nodeName.c_str(), node->nodeName.size());
    IO::write(stream, node->type);

    stream.write(reinterpret_cast<char*>(node->inSockets.size()), sizeof(size_t));
    for(const DSocket *socket : node->inSockets) {
        IO::write(stream, socket);
    }

    stream.write(reinterpret_cast<char*>(node->outSockets.size()), sizeof(size_t));
    for(const DSocket *socket : node->inSockets) {
        IO::write(stream, socket);
    }

    //TODO: write container data stuff
}

DNode::DNode(std::string name)
    : selected(false),
      space(0),
      varsocket(0),
      lastsocket(0),
      varcnt(0),
      ID(count++),
      nodeName(Project::instance()->registerItem(this, name.c_str())),
      _signalLiveTime(new Signal::LiveTimeTracker(this))
{
};

DNode::DNode(const DNode& node)
    : selected(false),
      space(0),
      varcnt(0),
      ID(count++),
      nodeName(Project::instance()->registerItem(this, node.getNodeName())),
      type(node.getType()),
      _signalLiveTime(new Signal::LiveTimeTracker(this))
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
    foreach(DinSocket *socket, getInSockets())
        delete socket;
    foreach(DoutSocket *socket, getOutSockets())
        delete socket;

    if(space)space->unregisterNode(this);

    Project::instance()->unregisterItem(this);
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

Vec2i DNode::getPos()const
{
    return pos;
}

void DNode::setPos(Vec2i value)
{
    pos = value;
    MT_CUSTOM_BOUND_SIGNAL_EMITTER(_signalLiveTime, "nodePositionChanged");
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

//void DNode::blockCB()    
//{
//    addSocketCallbacks.setBlock(true);
//}
//
//void DNode::unblockCB()    
//{
//    addSocketCallbacks.setBlock(false);
//}
//
//void DNode::blockRegCB()    
//{
//    blockCBregister = true; 
//}
//
//void DNode::unblockRegCB()    
//{
//    blockCBregister = false;
//}
//
NodeType DNode::getType()const
{
    return type;
}

void DNode::setType(NodeType value)
{
    type = NodeType(value);
}

DNode* DNode::clone()
{
    return new DNode(*this);
}

NodeList DNode::copy(NodeList nodes)    
{
   NodeList nodeCopies;
   for(auto *node : nodes)
        nodeCopies.push_back(node->clone());
   CopySocketMapper::remap();
   return nodeCopies;
}

NodeList DNode::getAllInNodes()
{
    NodeList nodes;
    nodes.push_back(this);
    for (auto socket : inSockets) {
        if(socket->toIn()->getCntdSocket()) {
            DNode *nextNode = socket->toIn()->getCntdSocket()->getNode();
            NodeList morenodes = nextNode->getAllInNodes();
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
    //if(type == CONTAINER
    //        ||type == CONDITIONCONTAINER
    //        ||type == FOR
    //        ||type == WHILE
    //        ||type == GATHER
    //        ||type == ILLUMINANCE
    //        ||type == ILLUMINATE
    //        ||type == SOLAR)
    //    return true;
    //else return false;
    return true;
}

//QDataStream &MindTree::operator <<(QDataStream &stream, DNode  *node)
//{
    ////stream<<node->getNodeName()<<node->getID()<<node->getType();
    //stream<<node->getPos();
    //stream<<(qint16)node->getInSockets().size()<<(qint16)node->getOutSockets().size();

    //foreach(DinSocket *socket, node->getInSockets())
    //    stream<<(DSocket*)socket;
    //foreach(DoutSocket *socket, node->getOutSockets())
    //    stream<<(DSocket*)socket;

    ////if(node->getNodeType() == FUNCTION) {
    ////    FunctionNode *fnode = (FunctionNode*) node;
    ////    stream<<fnode->getFunctionName();
    ////}

    //if(node->isContainer()) {
    //    ContainerNode *cnode =(ContainerNode*) node;
    //    stream<<cnode->getInputs()->getID()<<cnode->getOutputs()->getID();
    //    stream<<cnode->getSocketMapSize();
    //    foreach(const DSocket *socket, cnode->getMappedSocketsOnContainer())
    //        stream<<socket->getID()<<cnode->getSocketInContainer(socket)->getID();
    //    stream<<cnode->getContainerData();
    //    if(LoopNode::isLoopNode(node)){
    //        LoopNode *lnode = node->getDerived<LoopNode>();
    //        stream<<lnode->getLoopedInputs()->getID();
    //    }
    //}

    ////if(node->getNodeType() == LOOPINSOCKETS) {

    ////    LoopSocketNode *lsnode = (LoopSocketNode*)node;
    ////    stream<<lsnode->getLoopedSocketsCount();
    ////    foreach(DSocket *socket, lsnode->getLoopedSockets())
    ////        stream<<socket->getID()<<lsnode->getPartnerSocket(socket)->getID();
    ////}
    //return stream;
//}

DNode_ptr DNode::newNode(std::string name, NodeType t, int insize, int outsize)
{
    DNode_ptr node = newNodeFactory[t.id()](); 
    node->setNodeName(name);
    return node;
}

//QDataStream &MindTree::operator >>(QDataStream &stream, DNode_ptr *node)
//{
//    QString name;
//    unsigned short ID;
//    qint16 insocketsize, outsocketsize;
//    int nodetype;
//    QPointF nodepos;
//    stream>>name>>ID>>nodetype>>nodepos;
//    stream>>insocketsize>>outsocketsize;
//
//    //DNode_ptr newnode = DNode::newNode(name.toStdString(), (NType)nodetype, insocketsize, outsocketsize);
//    //LoadNodeIDMapper::setID(newnode, ID);
//    //newnode->setPos(nodepos);
//    //*node = newnode;
//
//    //DSocket *socket;
//    //newnode->blockCB();
//    //newnode->blockRegCB();
//    //for(int i=0; i<insocketsize; i++)
//    //{
//    //    socket = new DinSocket("", VARIABLE, newnode.get());
//    //    stream>>&socket;
//    //}
//    //for(int j=0; j<outsocketsize; j++)
//    //{
//    //    socket = new DoutSocket("", VARIABLE, newnode.get());
//    //    stream>>&socket;
//    //}
//    ////newnode->unblockCB();
//    ////newnode->unblockRegCB();
//
//    //if(newnode->getNodeType() == FUNCTION)
//    //{
//    //    FunctionNode *fnode = newnode->getDerived<FunctionNode>();
//    //    QString fname;
//    //    stream>>fname;
//    //    fnode->setFunctionName(fname.toStdString());
//    //}
//
//    //unsigned short inSocketID, outSocketID, keyID, valueID;
//    //int smapsize;
//    //if(newnode->isContainer())
//    //{
//    //    ContainerNode *contnode = newnode->getDerived<ContainerNode>();
//    //    stream>>inSocketID>>outSocketID;
//    //    stream>>smapsize;
//    //    auto cont_socket_map_ID_mapper = new QPair<unsigned short, unsigned short>[smapsize];
//    //    for(int i = 0; i < smapsize; i++)
//    //    {
//    //        stream>>keyID>>valueID;
//    //        cont_socket_map_ID_mapper[i].first = keyID;
//    //        cont_socket_map_ID_mapper[i].second = valueID;
//    //    }
//    //    ContainerSpace *space = 0;
//    //    stream>>&space;
//    //    contnode->setContainerData(space);
//    //    SocketNode *innode = LoadNodeIDMapper::getNode(inSocketID)->getDerived<SocketNode>();
//    //    contnode->setInputs(innode);
//    //    SocketNode *outnode = LoadNodeIDMapper::getNode(outSocketID)->getDerived<SocketNode>();
//    //    contnode->setOutputs(outnode);
//    //    if(LoopNode::isLoopNode(newnode.get())) {
//    //        unsigned short loopedNodeID;
//    //        stream >> loopedNodeID;
//    //        LoopSocketNode *loopedNode = LoadNodeIDMapper::getNode(loopedNodeID)->getDerived<LoopSocketNode>();
//    //        LoopSocketNode *loutnode = outnode->getDerived<LoopSocketNode>();
//    //        LoopSocketNode *linnode = innode->getDerived<LoopSocketNode>();
//    //        LoopNode *lnode = contnode->getDerived<LoopNode>();
//    //        lnode->setLoopedSockets(loopedNode);
//    //        loutnode->setPartner(loopedNode);
//    //        loopedNode->setPartner(loutnode);
//    //    }
//
//    //    for(int j = 0; j < smapsize; j++) {
//    //        keyID = cont_socket_map_ID_mapper[j].first;
//    //        valueID = cont_socket_map_ID_mapper[j].second;
//    //        contnode->mapOnToIn(const_cast<DSocket*>(LoadSocketIDMapper::getSocket(keyID)),
//    //                            const_cast<DSocket*>(LoadSocketIDMapper::getSocket(valueID)));
//    //    }
//    //}
//
//    //if(newnode->getNodeType() == LOOPINSOCKETS)
//    //        //||newnode->getNodeType() == LOOPOUTSOCKETS)
//    //{
//    //    LoopSocketNode *lsnode = newnode->getDerived<LoopSocketNode>();
//    //    int socketID, partnerID;
//    //    qint16 partnerSockets; 
//    //    stream>>partnerSockets;
//    //    for(int i = 0; i < partnerSockets; i++)
//    //    {
//    //        stream>>socketID>>partnerID;
//    //        DSocket *socket = const_cast<DSocket*>(LoadSocketIDMapper::getSocket(socketID));
//    //        DSocket *partner = const_cast<DSocket*>(LoadSocketIDMapper::getSocket(partnerID));
//    //        lsnode->mapPartner(socket, partner);
//    //    }
//    //}
//
//    ////if(MathNode::isMathNode(newnode)
//    ////    ||newnode->getNodeType() == GETARRAY
//    ////    ||newnode->getNodeType() == SETARRAY
//    ////    ||newnode->getNodeType() == VARNAME) {
//    ////    DinSocket *in = newnode->getInSockets().first();
//    ////    DoutSocket *out = newnode->getOutSockets().first();
//    ////}
//
//
//    return stream;
//}

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

void DNode::setNodeType(NodeType t)    
{
    type = t;
}

void DNode::setNodeName(std::string name)
{
    Project::instance()->unregisterItem(this);
    nodeName = Project::instance()->registerItem(this, name);
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
}

void DNode::dec_var_socket(DSocket *socket)
{
    removeSocket(socket);
    varcnt -= 1;
}

void DNode::inc_var_socket()
{
    lastsocket = varsocket;
    if(lastsocket->getDir() == DSocket::IN)
        varsocket = new DinSocket("+", "VARIABLE", this);
    else
        varsocket = new DoutSocket("+", "VARIABLE", this);
    varsocket->setVariable(true);
    varcnt +=1;
}

void DNode::clearSocketLinks()
{
    foreach(DinSocket *socket, getInSockets())
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
    DSocket *varsocket = 0;
    if(dir == DSocket::IN)
        varsocket = new DinSocket("+", "VARIABLE", this);
    else
        varsocket = new DoutSocket("+", "VARIABLE", this);
    varsocket->setVariable(true);
}

