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

bool DNode::isInput(const DNode *node)
{
    //if(node->getNodeType() == SURFACEINPUT
    //   ||node->getNodeType() == DISPLACEMENTINPUT
    //   ||node->getNodeType() == VOLUMEINPUT
    //   ||node->getNodeType() == LIGHTINPUT)
    //    return true;
    //else
        return false;
}

DNode *DNode::dropNode(std::string filepath)
{
    //QFile file(filepath.c_str());
    //file.open(QIODevice::ReadOnly);
    //QDataStream stream(&file);

    //DNode_ptr node = 0;
    //FRG_NODE_HEADER_CHECK
    //{
    //    stream>>&node;

    //    file.close();

    //    LoadNodeIDMapper::clear();
    //    LoadSocketIDMapper::remap();

    //}
    //return node.get();
}

//ContainerNode *DNode::buildContainerNode(QList<DNode*>nodes)
//{
//    int SPACING = 200;
//
//    if(nodes.isEmpty()) return 0;
//    ContainerNode *contnode = new ContainerNode("New Node", false);
//
//    QList<DNodeLink*>ins = FRG::CurrentProject->getInLinks(FRG::Space->selectedNodes());
//    QList<DNodeLink*>outs = FRG::CurrentProject->getOutLinks(FRG::Space->selectedNodes());
//
//    FRG::CurrentProject->setNodePosition(contnode, FRG::Space->getCenter(nodes));
//    FRG::Space->centerNodes(nodes);
//    float minX = 0, maxX = 0;
//    foreach(DNode *node, nodes)
//    {
//        int nodeWidth = 50;
//        float nodeX = node->getPos().x();
//        float nodeMinX = nodeX - nodeWidth/2;
//        float nodeMaxX = nodeX + nodeWidth/2;
//        FRG::SpaceDataInFocus->unregisterNode(node);
//        contnode->getContainerData()->addNode(node);
//        if(minX > nodeMinX) minX = nodeMinX;
//        if(maxX < nodeMaxX) maxX = nodeMaxX;
//    }
//
//    FRG::CurrentProject->setNodePosition(contnode->getInputs(), QPointF(minX - SPACING, 0));
//    FRG::CurrentProject->setNodePosition(contnode->getOutputs(), QPointF(maxX + SPACING, 0));
//    foreach(DNodeLink *nld, ins)
//    {
//        //relink the first input of the slected nodes to the container input node
//        DoutSocket *entryInContainer = static_cast<DoutSocket*>(contnode->getInputs()->getVarSocket());
//        nld->in->addLink(entryInContainer);
//
//        //link the generated mapped socket of the container input to the output previously conntected
//        DinSocket *containerInput = static_cast<DinSocket*>(contnode->getSocketOnContainer(entryInContainer));
//        containerInput->addLink(nld->out);
//    }
//
//    foreach(DNodeLink *nld, outs)
//    {
//        //link the container output node to the last output of the selected nodes
//        DinSocket *exitInContainer = contnode->getOutputs()->getVarSocket()->toIn();
//        foreach(DinSocket *output, contnode->getOutputs()->getInSockets())
//            if(output->getCntdSocket() == nld->out)
//            {
//                exitInContainer = output;
//                break;
//            }
//        exitInContainer->addLink(nld->out);
//
//        //relink the input previously connected to the generated mapped socket of the output of the container
//        nld->in->addLink(contnode->getSocketOnContainer(exitInContainer)->toOut());
//    }
//
//    FRG::SpaceDataInFocus->addNode(contnode);
//    return contnode;
//}
//
//void DNode::unpackContainerNode(DNode *node)
//{
//    ContainerNode *contnode = node->getDerived<ContainerNode>();
//    QList<DNodeLink*>ins = FRG::CurrentProject->getInLinks(contnode);
//    QList<DNodeLink*>outs = FRG::CurrentProject->getOutLinks(contnode);
//
//    QList<DNodeLink> newInLinks;
//    QList<DNodeLink> newOutLinks;
//
//    foreach(DNodeLink *in, ins)
//    {
//        in->out->unregisterSocket(in->in);
//        DoutSocket *inContainer = contnode->getSocketInContainer(in->in)->toOut();
//        QList<DNodeLink> TMPnewInLinks = inContainer->getLinks();
//        foreach(DNodeLink dnlink, TMPnewInLinks)
//        {
//            newInLinks.append(DNodeLink(dnlink.in, in->out));
//            inContainer->unregisterSocket(dnlink.in, false);
//        }
//    }
//
//    foreach(DNodeLink *out, outs)
//    {
//        out->out->unregisterSocket(out->in);
//        DinSocket *inContainer = contnode->getSocketInContainer(out->out)->toIn();
//        DoutSocket *newOutSocket = inContainer->getCntdSocket();
//        newOutSocket->unregisterSocket(inContainer, false);
//        newOutLinks.append(DNodeLink(out->in, newOutSocket));
//    }
//
//    QList<DNode*>nodes(contnode->getContainerData()->getNodes());
//    QPointF contNodePos(contnode->getPos());
//    foreach(DNode *node, nodes)
//    {
//        if(node == contnode->getInputs()
//            ||node == contnode->getOutputs())
//            continue;
//
//        node->getSpace()->unregisterNode(node);
//        FRG::SpaceDataInFocus->addNode(node);
//        QPointF oldPos(node->getPos());
//        QPointF newPos = contNodePos + oldPos;
//        FRG::CurrentProject->setNodePosition(node, newPos);
//    }
//
//    foreach(DNodeLink in, newInLinks)
//        in.in->setCntdSocket(in.out);
//
//    foreach(DNodeLink out, newOutLinks)
//        out.in->setCntdSocket(out.out);
//
//    FRG::SpaceDataInFocus->unregisterNode(node);
//}

ContainerNode::ContainerNode(std::string name, bool raw)
    : DNode(name), containerData(0)
{
    setType("CONTAINER");
    if(!raw)
    {
        setContainerData(new ContainerSpace);
        containerData->setName(name);
        new SocketNode(DSocket::IN, this);
        new SocketNode(DSocket::OUT, this);
    }
}

ContainerNode::ContainerNode(const ContainerNode &node)
    : DNode(node)
{
    setNodeType("CONTAINER");
    setContainerData(new ContainerSpace(*node.getContainerData()));

    for(const DSocket *socket : node.getMappedSocketsOnContainer())
        mapOnToIn(CopySocketMapper::getCopy(socket), 
                  CopySocketMapper::getCopy(node.getSocketInContainer(socket)));

    setInputs(CopyNodeMapper::getCopy(node.getInputs()));
    setOutputs(CopyNodeMapper::getCopy(node.getOutputs()));
}

ContainerNode::~ContainerNode()
{
    if(containerData) delete containerData;
}

NodeList ContainerNode::getAllInNodes()
{
    NodeList nodes = DNode::getAllInNodes();
    for(auto socket : getOutputs()->getInSockets()) {
        DoutSocket *cntd = nullptr;
        if((cntd = socket->toIn()->getCntdSocket())) {
            NodeList morenodes = cntd->getNode()->getAllInNodes();
            nodes.insert(nodes.end(), morenodes.begin(), morenodes.end());
        }
    }
    return nodes;
}

ConstNodeList ContainerNode::getAllInNodesConst() const
{
    ConstNodeList nodes = DNode::getAllInNodesConst();
    for(auto socket : getOutputs()->getInSockets()) {
        DoutSocket *cntd = nullptr;
        if((cntd = socket->toIn()->getCntdSocket())) {
            ConstNodeList morenodes = cntd->getNode()->getAllInNodesConst();
            nodes.insert(nodes.end(), morenodes.begin(), morenodes.end());
        }
    }
    return nodes;
}

void ContainerNode::addMappedSocket(DSocket *socket)
{
    DSocket *mapped_socket = 0;
    if(socket->getDir() == DSocket::IN)
        mapped_socket = new DoutSocket(socket->getName(), 
                                       socket->getType(), 
                                       inSocketNode);
    else
        mapped_socket = new DinSocket(socket->getName(), 
                                      socket->getType(), 
                                      outSocketNode);
    mapOnToIn(socket, mapped_socket);     
}

void ContainerNode::addtolib()
{
    //QDir::setCurrent(QCoreApplication::applicationDirPath());

    //QString filename;
    //filename.append("nodes/");
    //filename.append(getNodeName().c_str());
    //filename.append(".node");
    //QFile file(filename);
    //file.open(QIODevice::WriteOnly);
    //QDataStream out(&file);
    //out<<FRG_NODE_HEADER;
    //out<<this;
}

void ContainerNode::addItems(NodeList nodes)
{
    for(auto *node : nodes)
        containerData->addNode(node);
}

void ContainerNode::setInputs(SocketNode *inputNode)
{
    inSocketNode = inputNode;
    auto nodes = containerData->getNodes();
    if(std::find(nodes.begin(), nodes.end(), inputNode) == nodes.end())
        containerData->addNode(inputNode);
    inputNode->connectToContainer(this);
}

void ContainerNode::setInputs(DNode *inputNode)    
{
    setInputs(static_cast<SocketNode*>(inputNode));
}

void ContainerNode::setOutputs(SocketNode *outputNode)
{
    outSocketNode = outputNode;
    containerData->addNode(outputNode);
    outputNode->connectToContainer(this);
}

void ContainerNode::setOutputs(DNode *outputNode)    
{
    setOutputs(static_cast<SocketNode*>(outputNode));
}

SocketNode* ContainerNode::getInputs() const
{
    return inSocketNode;
}

SocketNode* ContainerNode::getOutputs() const
{
    return outSocketNode;
}

void ContainerNode::newSocket(DSocket *socket)
{
    DSocket *mapped_socket = 0;
    if(socket->getDir() == DSocket::IN)
        mapped_socket = new DoutSocket(*socket->toOut(), this);
    else
        mapped_socket = new DinSocket(*socket->toIn(), this);
    mapOnToIn(socket, mapped_socket);
}

void ContainerNode::killSocket(DSocket *socket)
{
    DSocket *contsocket = const_cast<DSocket*>(socket_map[socket]);
    socket_map.erase(socket);
    removeSocket(contsocket);
}

DSocket *ContainerNode::getSocketInContainer(DSocket *socket)
{
    return const_cast<DSocket*>(socket_map[ const_cast<DSocket*>(socket) ]);
}

const DSocket *ContainerNode::getSocketInContainer(const DSocket *socket) const
{
    return socket_map.at(socket);
}

void ContainerNode::mapOnToIn(const DSocket *on, const DSocket *in)    
{
    if(!on &&!in) return;
    socket_map.insert({on, in});
    //DSocket::nameCB(on, in);
    //DSocket::typeCB(on, in);
    //DSocket::nameCB(in, on);
    //DSocket::typeCB(in, on);
}

DSocket *ContainerNode::getSocketOnContainer(DSocket *socket)
{
    for(auto p : socket_map)
        if (p.second == socket)
            return const_cast<DSocket*>(p.first);
}

const DSocket *ContainerNode::getSocketOnContainer(const DSocket *socket) const
{
    for(auto p : socket_map)
        if (p.second == socket)
            return p.first;
}

std::vector<const DSocket*> ContainerNode::getMappedSocketsOnContainer() const
{
    std::vector<const DSocket*> sockets;
    for (auto p : socket_map) sockets.push_back(p.first);
    return sockets;
}

int ContainerNode::getSocketMapSize() const
{
    return socket_map.size();
}

void ContainerNode::setNodeName(std::string name)
{
    DNode::setNodeName(name);
    if(containerData)containerData->setName(name.c_str());
}

ContainerSpace* ContainerNode::getContainerData() const
{
    return containerData;
}

void ContainerNode::setContainerData(ContainerSpace* value)
{
    containerData = value;
    containerData->setContainer(this);
}

bool ContainerNode::operator==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    if(*containerData != *node.getDerivedConst<ContainerNode>()->containerData)
        return false;
    return true;
}

bool ContainerNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

//ConditionContainerNode::ConditionContainerNode(bool raw)
//    : ContainerNode("Condition", raw)
//{
//    setNodeType(CONDITIONCONTAINER);
//    if(!raw)
//    {
//        new DinSocket("Condition", CONDITION, this);
//    }
//}
//
//ConditionContainerNode::ConditionContainerNode(const ConditionContainerNode* node)
//    : ContainerNode(node)
//{
//    setNodeType(CONDITIONCONTAINER);
//}
//
SocketNode::SocketNode(DSocket::SocketDir dir, ContainerNode *contnode, bool raw)
    : container(0)
{
    if (dir == DSocket::IN)
    {
        setType("INSOCKETS");
        if(!raw && contnode)setInSocketNode(contnode);
    }
    else
    {
        setType("OUTSOCKETS");
        if(!raw && contnode)setOutSocketNode(contnode);
    }
}

SocketNode::SocketNode(const SocketNode &node)
    : DNode(node), 
    container(CopyNodeMapper::getCopy(static_cast<DNode*>(node.getContainer()))
              ->getDerived<ContainerNode>())
{
}

void SocketNode::setInSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(DSocket::OUT);
    setNodeName("Input");
    contnode->setInputs(this);
}

void SocketNode::setOutSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(DSocket::IN);
    setNodeName("Output");
    contnode->setOutputs(this);
}

void SocketNode::connectToContainer(ContainerNode *contnode)
{
    container = contnode;
}

ContainerNode* SocketNode::getContainer() const
{
    return container;
}

void SocketNode::inc_var_socket()
{
    DNode::inc_var_socket();
	DSocket *newsocket;
//    unsigned short arrID = container->getSocketOnContainer(getLastSocket())->getArrayID();
	if(getLastSocket()->getDir() == DSocket::IN)
		newsocket = new DoutSocket(getLastSocket()->getName(), getLastSocket()->getType(), container);
	else
		newsocket = new DinSocket(getLastSocket()->getName(), getLastSocket()->getType(), container);
    container->mapOnToIn(newsocket, getLastSocket());
 //   if(arrID > 0)
 //       newsocket->setArray(arrID);
}

void SocketNode::dec_var_socket(DSocket *socket)
{
    if(container)container->removeSocket(container->getSocketOnContainer(socket));
    DNode::dec_var_socket(socket);
}

//LoopSocketNode::LoopSocketNode(socket_dir dir, LoopNode *contnode, bool raw)
//    :SocketNode(dir, contnode, true), partner(0)
//{
//    if (dir == IN) {
//        setNodeType(LOOPINSOCKETS);
//        if(!raw && contnode) {
//            //setDynamicSocketsNode(OUT);
//            setNodeName("Looped Sockets");
//        }
//    }
//    else {
//        setNodeType(LOOPOUTSOCKETS);
//        if(!raw && contnode)setOutSocketNode(contnode);
//    }
//}
//
//LoopSocketNode::LoopSocketNode(const LoopSocketNode* node)
//    : SocketNode(node), partner(0)
//{
//    foreach(DSocket *original, node->getLoopedSockets())
//        loopSocketMap.insert(const_cast<DSocket*>(CopySocketMapper::getCopy(original)),
//                             const_cast<DSocket*>(CopySocketMapper::getCopy(node->getPartnerSocket(original))));        
//}
//
//void LoopSocketNode::dec_var_socket(DSocket *socket)
//{
//    SocketNode::dec_var_socket(socket);
//    if(partner != 0)
//    {
//        LoopSocketNode *tmpp = partner;
//        partner->setPartner(0);
//        deletePartnerSocket(socket);
//        partner->setPartner(tmpp);
//    }
//}
//
//QList<DSocket*> LoopSocketNode::getLoopedSockets() const
//{
//    return loopSocketMap.keys();
//}
//
//qint16 LoopSocketNode::getLoopedSocketsCount() const
//{
//    return loopSocketMap.size();
//}
//
//void LoopSocketNode::createPartnerSocket(DSocket *socket)
//{
//	DSocket *partnerSocket;
//	DSocket *newsocket;
//	if(socket->getDir() == IN)
//    {
//        partnerSocket = new DoutSocket(socket->getName(), socket->getType(), partner);
//		newsocket = new DinSocket(socket->getName(), socket->getType(), getContainer());
//        getContainer()->mapOnToIn(newsocket, partnerSocket);
//    }
//	else
//		partnerSocket = new DinSocket(socket->getName(), socket->getType(), partner);
//    loopSocketMap.insert(socket, partnerSocket);
//    //partner->mapPartner(partnerSocket, socket);
//}
//
//void LoopSocketNode::deletePartnerSocket(DSocket *socket)
//{
//    DinSocket *partnerSocket = getPartnerSocket(socket)->toIn();
//    partner->removeSocket(partnerSocket);
//    loopSocketMap.remove(socket);
//}
//
//void LoopSocketNode::mapPartner(DSocket* here, DSocket *partner)    
//{
//   loopSocketMap.insert(here, partner); 
//}
//
//void LoopSocketNode::setPartner(LoopSocketNode *p)
//{
//    partner = p;
//}
//
//DSocket *LoopSocketNode::getPartnerSocket(const DSocket *socket) const
//{
//    if(loopSocketMap.keys().contains(const_cast<DSocket*>(socket)))
//        return loopSocketMap.value(const_cast<DSocket*>(socket));
//    else return 0;
//}
//
//void LoopSocketNode::inc_var_socket()
//{
//    SocketNode::inc_var_socket();
//    if(getNodeType() == LOOPOUTSOCKETS)
//        createPartnerSocket(getLastSocket());
//}
//
//LoopNode::LoopNode(std::string name, bool raw)
//    : ContainerNode(name, true)
//{
//    if(!raw)
//    {
//        LoopSocketNode *loutNode, *linNode, *loopNode;
//        setContainerData(new ContainerSpace);
//        getContainerData()->setName(name.c_str());
//
//        loopNode = new LoopSocketNode(IN, this);
//        setLoopedSockets(loopNode);
//        linNode = new LoopSocketNode(IN, this);
//        setInputs(linNode);
//        linNode->setDynamicSocketsNode(OUT);
//        loutNode = new LoopSocketNode(OUT, this);
//
//        loopNode->setPartner(loutNode);
//        loutNode->setPartner(loopNode);
//    }
//}
//
//LoopNode::LoopNode(const LoopNode* node)
//    : ContainerNode(node)
//{
//}
//
LoopSocketNode* LoopNode::getLoopedInputs()    
{
    return loopSockets; 
}

//void LoopNode::setLoopedSockets(LoopSocketNode *node)    
//{
//    loopSockets = node;
//    if(!getContainerData()->getNodes().contains(node))
//        getContainerData()->addNode(node);
//    node->connectToContainer(this);
//}
//
bool LoopNode::isLoopNode(DNode *node)
{
    return false;
    //return (node->getNodeType() == FOR
    //        ||node->getNodeType() == WHILE
    //        ||node->getNodeType() == ILLUMINANCE
    //        ||node->getNodeType() == ILLUMINATE
    //        ||node->getNodeType() == SOLAR
    //        ||node->getNodeType() == GATHER);
}
//WhileNode::WhileNode(bool raw)
//    : LoopNode("While", raw)
//{
//    setNodeType(WHILE);
//    if(!raw)
//    {
//        new DinSocket("Condition", CONDITION, getOutputs());
//        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
//        inputSocketList->move(0, inputSocketList->len() - 1);
//    }
//}
//
//WhileNode::WhileNode(const WhileNode* node)
//    : LoopNode(node)
//{
//}
//
//ForNode::ForNode(bool raw)
//    : LoopNode("For", raw)
//{
//    setNodeType(FOR);
//    if(!raw)
//    {
//        addMappedSocket(new DinSocket("Start", INTEGER, this));
//        addMappedSocket(new DinSocket("End", INTEGER, this));
//        addMappedSocket(new DinSocket("Step", INTEGER, this));
//        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
//        inputSocketList->move(0, inputSocketList->len() - 1);
//    }
//}
//
//ForNode::ForNode(const ForNode* node)
//    : LoopNode(node)
//{
//}
//
//ForeachNode::ForeachNode(bool raw)
//    : ContainerNode("Foreach", raw)
//{
//    setNodeType(FOREACHNODE);
//    if(!raw)
//    {
//        new DinSocket("Array", VARIABLE, this);
//        new DoutSocket("Array", VARIABLE, this);
//        new DoutSocket("Index", INTEGER, getInputs());
//        new DinSocket("Value", VARIABLE, getOutputs());
//        DSocketList *inputSocketList = getInputs()->getDerived<SocketNode>()->getOutSocketLlist();
//        DSocketList *outSocketList = getOutputs()->getDerived<SocketNode>()->getInSocketLlist();
//        inputSocketList->move(0, inputSocketList->len() - 1);
//        outSocketList->move(0, outSocketList->len() - 1);
//    }
//}
//
//ForeachNode::ForeachNode(const ForeachNode* node)
//    : ContainerNode(node)
//{
//}
