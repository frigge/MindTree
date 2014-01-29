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


#include "QCoreApplication"
#include "QTextStream"
#include "QDir"
#include "QDebug"
#include "QProcess"

#include "data/frg.h"
#include "data/dnspace.h"
#include "data/project.h"
#include "iostream"
#include "data/properties.h"

#include "data/signal.h"

#include "data_node.h"

using namespace MindTree;

unsigned short DNode::count = 1;
QHash<unsigned short, DNode_ptr>LoadNodeIDMapper::loadIDMapper;
QHash<DNode*, DNode*> CopyNodeMapper::nodeMap;
std::vector<std::function<DNode_ptr()>> DNode::newNodeFactory;

unsigned short LoadNodeIDMapper::getID(DNode_ptr node)
{
    return loadIDMapper.key(node);
}

void LoadNodeIDMapper::setID(DNode_ptr node, unsigned short ID)    
{
    loadIDMapper.insert(ID, node);
}

DNode_ptr LoadNodeIDMapper::getNode(unsigned short ID)    
{
    return loadIDMapper.value(ID);
}

void LoadNodeIDMapper::clear()    
{
    loadIDMapper.clear();
}

void CopyNodeMapper::setNodePair(DNode *original, DNode *copy)    
{
   nodeMap.insert(original, copy); 
}

DNode * CopyNodeMapper::getCopy(DNode *original)    
{
    if(!nodeMap.contains(original)) return 0;
    return nodeMap.value(original);
}

DNode::DNode(std::string name)
    : space(0),
      selected(false),
      varcnt(0),
      ID(count++),
      nodeName(FRG::CurrentProject->registerItem(this, name.c_str()).toStdString()),
      varsocket(0),
      lastsocket(0),
      blockCBregister(false)
{
};

DNode::DNode(const DNode* node)
    : space(0),
      selected(false),
      varcnt(0),
      ID(count++),
      nodeName(node->getNodeName()),
      type(node->getNodeType()),
      blockCBregister(false)
{
    for(auto prop : node->getProperties())
        properties[prop.first] = Property(prop.second);

    foreach(DinSocket *socket, node->getInSockets())
        new DinSocket(*socket, this);
    foreach(DoutSocket *socket, node->getOutSockets())
        new DoutSocket(*socket, this);
    varsocket = const_cast<DSocket*>(CopySocketMapper::getCopy(node->getVarSocket()));
    lastsocket = const_cast<DSocket*>(CopySocketMapper::getCopy(node->getLastSocket()));

    CopyNodeMapper::setNodePair(const_cast<DNode*>(node), this);
    setPos(node->getPos());
}

DNode::DNode(const DNode& node)
    : space(0),
      selected(false),
      varcnt(0),
      ID(count++),
      nodeName(node.getNodeName()),
      type(node.getType()),
      blockCBregister(false)
{
    for(auto prop : node.getProperties())
        properties[prop.first] = prop.second;

    foreach(DinSocket *socket, node.getInSockets())
        new DinSocket(*socket, this);
    foreach(DoutSocket *socket, node.getOutSockets())
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

    emit deleted(this);
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

const Property DNode::getProperty(std::string name)const
{
    const auto prop = properties[name];
    return prop;
}

Property DNode::getProperty(std::string name)
{
    return properties[name];
}

PropertyMap DNode::getProperties()   const 
{
    return properties;
}

Property DNode::operator[](std::string name)
{
    return properties[name];
}

void DNode::setProperty(Property value)
{
    auto name = value.getName();
    auto prop = properties.find(name);
    if(prop != properties.end()) {
        properties.erase(name);
    }

    properties[value.getName()] = value;
}

void DNode::rmProperty(std::string name)    
{
    properties.erase(name);
}

QPointF DNode::getPos()const
{
    return pos;
}

void DNode::setPos(QPointF value)
{
    pos = value;
}

DSocket* DNode::getSocketByIDName(std::string idname)    
{
    foreach(DinSocket *socket, getInSockets())
        if(socket->getIDName() == idname)
            return socket;
    foreach(DoutSocket *socket, getOutSockets())
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

void DNode::setType(std::string value)
{
    type = NodeType(value);
}

DNode* DNode::clone()
{
    return new DNode(*this);
}

QList<DNode*> DNode::copy(QList<DNode*>nodes)    
{
   QList<DNode*>nodeCopies;
   foreach(DNode *node, nodes)
        nodeCopies.append(node->clone());
   CopySocketMapper::remap();
   return nodeCopies;
}

NodeList DNode::getAllInNodes(NodeList nodes) 
{
    nodes.append(this);
    LLsocket *tmp = inSockets.getFirst();
    if(!tmp)return nodes;
    do{
        if(tmp->socket->getArray())
            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())
                nodes = socket->getNode()->getAllInNodes(nodes);
        else if(tmp->socket->toIn()->getCntdSocket())
            nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodes(nodes);
    }while(tmp = tmp->next);
    return nodes;
}

ConstNodeList DNode::getAllInNodesConst(ConstNodeList nodes) const
{
    nodes.append(this);
    LLsocket *tmp = inSockets.getFirst();
    if(!tmp)return nodes;
    do{
        if(tmp->socket->getArray())
            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())
                nodes = socket->getNode()->getAllInNodesConst(nodes);
        else if(tmp->socket->toIn()->getCntdSocket())
            nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodesConst(nodes);
    }while(tmp = tmp->next);
    return nodes;
}

bool DNode::isContainer() const
{
    if(type == CONTAINER
            ||type == CONDITIONCONTAINER
            ||type == FOR
            ||type == WHILE
            ||type == GATHER
            ||type == ILLUMINANCE
            ||type == ILLUMINATE
            ||type == SOLAR)
        return true;
    else return false;
}

QDataStream &MindTree::operator <<(QDataStream &stream, DNode  *node)
{
    stream<<node->getNodeName()<<node->getID()<<node->getNodeType();
    stream<<node->getPos();
    stream<<(qint16)node->getInSockets().size()<<(qint16)node->getOutSockets().size();

    foreach(DinSocket *socket, node->getInSockets())
        stream<<(DSocket*)socket;
    foreach(DoutSocket *socket, node->getOutSockets())
        stream<<(DSocket*)socket;

    if(node->getNodeType() == FUNCTION) {
        FunctionNode *fnode = (FunctionNode*) node;
        stream<<fnode->getFunctionName();
    }

    if(node->isContainer()) {
        ContainerNode *cnode =(ContainerNode*) node;
        stream<<cnode->getInputs()->getID()<<cnode->getOutputs()->getID();
        stream<<cnode->getSocketMapSize();
        foreach(const DSocket *socket, cnode->getMappedSocketsOnContainer())
            stream<<socket->getID()<<cnode->getSocketInContainer(socket)->getID();
        stream<<cnode->getContainerData();
        if(LoopNode::isLoopNode(node)){
            LoopNode *lnode = node->getDerived<LoopNode>();
            stream<<lnode->getLoopedInputs()->getID();
        }
    }

    if(node->getNodeType() == LOOPINSOCKETS) {

        LoopSocketNode *lsnode = (LoopSocketNode*)node;
        stream<<lsnode->getLoopedSocketsCount();
        foreach(DSocket *socket, lsnode->getLoopedSockets())
            stream<<socket->getID()<<lsnode->getPartnerSocket(socket)->getID();
    }
    return stream;
}

DNode_ptr DNode::newNode(std::string name, NType t, int insize, int outsize)
{
    DNode_ptr node = newNodeFactory[NodeType(t).id()](); 
    node->setNodeName(name);
    node->setNodeType(t);
    return node;
}

QDataStream &MindTree::operator >>(QDataStream &stream, DNode_ptr *node)
{
    QString name;
    unsigned short ID;
    qint16 insocketsize, outsocketsize;
    int nodetype;
    QPointF nodepos;
    stream>>name>>ID>>nodetype>>nodepos;
    stream>>insocketsize>>outsocketsize;

    DNode_ptr newnode = DNode::newNode(name.toStdString(), (NType)nodetype, insocketsize, outsocketsize);
    LoadNodeIDMapper::setID(newnode, ID);
    newnode->setPos(nodepos);
    *node = newnode;

    DSocket *socket;
    //newnode->blockCB();
    //newnode->blockRegCB();
    for(int i=0; i<insocketsize; i++)
    {
        socket = new DinSocket("", VARIABLE, newnode.get());
        stream>>&socket;
    }
    for(int j=0; j<outsocketsize; j++)
    {
        socket = new DoutSocket("", VARIABLE, newnode.get());
        stream>>&socket;
    }
    //newnode->unblockCB();
    //newnode->unblockRegCB();

    if(newnode->getNodeType() == FUNCTION)
    {
        FunctionNode *fnode = newnode->getDerived<FunctionNode>();
        QString fname;
        stream>>fname;
        fnode->setFunctionName(fname.toStdString());
    }

    unsigned short inSocketID, outSocketID, keyID, valueID;
    int smapsize;
    if(newnode->isContainer())
    {
        ContainerNode *contnode = newnode->getDerived<ContainerNode>();
        stream>>inSocketID>>outSocketID;
        stream>>smapsize;
        auto cont_socket_map_ID_mapper = new QPair<unsigned short, unsigned short>[smapsize];
        for(int i = 0; i < smapsize; i++)
        {
            stream>>keyID>>valueID;
            cont_socket_map_ID_mapper[i].first = keyID;
            cont_socket_map_ID_mapper[i].second = valueID;
        }
        ContainerSpace *space = 0;
        stream>>&space;
        contnode->setContainerData(space);
        SocketNode *innode = LoadNodeIDMapper::getNode(inSocketID)->getDerived<SocketNode>();
        contnode->setInputs(innode);
        SocketNode *outnode = LoadNodeIDMapper::getNode(outSocketID)->getDerived<SocketNode>();
        contnode->setOutputs(outnode);
        if(LoopNode::isLoopNode(newnode.get())) {
            unsigned short loopedNodeID;
            stream >> loopedNodeID;
            LoopSocketNode *loopedNode = LoadNodeIDMapper::getNode(loopedNodeID)->getDerived<LoopSocketNode>();
            LoopSocketNode *loutnode = outnode->getDerived<LoopSocketNode>();
            LoopSocketNode *linnode = innode->getDerived<LoopSocketNode>();
            LoopNode *lnode = contnode->getDerived<LoopNode>();
            lnode->setLoopedSockets(loopedNode);
            loutnode->setPartner(loopedNode);
            loopedNode->setPartner(loutnode);
        }

        for(int j = 0; j < smapsize; j++) {
            keyID = cont_socket_map_ID_mapper[j].first;
            valueID = cont_socket_map_ID_mapper[j].second;
            contnode->mapOnToIn(const_cast<DSocket*>(LoadSocketIDMapper::getSocket(keyID)),
                                const_cast<DSocket*>(LoadSocketIDMapper::getSocket(valueID)));
        }
    }

    if(newnode->getNodeType() == LOOPINSOCKETS)
            //||newnode->getNodeType() == LOOPOUTSOCKETS)
    {
        LoopSocketNode *lsnode = newnode->getDerived<LoopSocketNode>();
        int socketID, partnerID;
        qint16 partnerSockets; 
        stream>>partnerSockets;
        for(int i = 0; i < partnerSockets; i++)
        {
            stream>>socketID>>partnerID;
            DSocket *socket = const_cast<DSocket*>(LoadSocketIDMapper::getSocket(socketID));
            DSocket *partner = const_cast<DSocket*>(LoadSocketIDMapper::getSocket(partnerID));
            lsnode->mapPartner(socket, partner);
        }
    }

    //if(MathNode::isMathNode(newnode)
    //    ||newnode->getNodeType() == GETARRAY
    //    ||newnode->getNodeType() == SETARRAY
    //    ||newnode->getNodeType() == VARNAME) {
    //    DinSocket *in = newnode->getInSockets().first();
    //    DoutSocket *out = newnode->getOutSockets().first();
    //}


    return stream;
}

bool DNode::operator==(const DNode &node)const
{
    if(getNodeType() != node.getNodeType())
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

void DNode::setNodeType(NType t)
{
    type = NodeType(t);
}

void DNode::setNodeType(NodeType t)    
{
    type = t;
}

void DNode::setNodeName(std::string name)
{
    FRG::CurrentProject->unregisterItem(this);
    nodeName = FRG::CurrentProject->registerItem(this, name.c_str()).toStdString();
}

void DNode::addSocket(DSocket *socket)
{
    setSocketIDName(socket);
    if(socket->getDir()==IN) inSockets.add(socket);
    else 
		outSockets.add(socket);

    addSocketCallbacks();
}

void DNode::setSocketIDName(DSocket *socket)    
{
    QStringList socketNames = getSocketNames();
    for(int i = 1; socketNames.contains(socket->getIDName().c_str()); i++)
        socket->setIDName(socket->getName() + std::to_string(i));
}

QStringList DNode::getSocketNames()    
{
    QStringList names;
    foreach(DinSocket *socket, getInSockets())
        names.append(socket->getIDName().c_str());
    foreach(DoutSocket *socket, getOutSockets())
        names.append(socket->getIDName().c_str());

    return names;
}

//void DNode::regAddSocketCB(Callback *cb)
//{
//    addSocketCallbacks.add(cb);
//}
//
//void DNode::remAddSocketCB(Callback *cb)
//{
//    addSocketCallbacks.remove(cb);
//}
//
void DNode::removeSocket(DSocket *socket)
{
    if(!socket)return;
    if(socket->getDir() == IN) {
        inSockets.rm(socket);
        delete socket;
    }
    else {
        outSockets.rm(socket);
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
    if(lastsocket->getDir() == IN)
        varsocket = new DinSocket("+", VARIABLE, this);
    else
        varsocket = new DoutSocket("+", VARIABLE, this);
    varsocket->setVariable(true);
    varcnt +=1;
}

void DNode::clearSocketLinks()
{
    foreach(DinSocket *socket, getInSockets())
       socket->clearLink();
}

bool DNode::isGhost()
{
    return ghost;
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
    return outSockets.returnAsOutSocketList();
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
    return inSockets.returnAsInSocketList();
}

DSocketList* DNode::getInSocketLlist()    const
{
    return &inSockets;
}

void DNode::setInSockets(DinSocketList value)
{
}

NType DNode::getNodeType() const
{
    return type.getOldType();
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

void DNode::setDynamicSocketsNode(socket_dir dir, socket_type t)
{
    DSocket *varsocket = 0;
    if(dir == IN)
        varsocket = new DinSocket("+", t, this);
    else
        varsocket = new DoutSocket("+", t, this);
    varsocket->setVariable(true);
}

bool DNode::isInput(const DNode *node)
{
    if(node->getNodeType() == SURFACEINPUT
       ||node->getNodeType() == DISPLACEMENTINPUT
       ||node->getNodeType() == VOLUMEINPUT
       ||node->getNodeType() == LIGHTINPUT)
        return true;
    else
        return false;
}

DNode *DNode::dropNode(std::string filepath)
{
    QFile file(filepath.c_str());
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    DNode_ptr node = 0;
    FRG_NODE_HEADER_CHECK
    {
        stream>>&node;

        file.close();

        LoadNodeIDMapper::clear();
        LoadSocketIDMapper::remap();

    }
    return node.get();
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

FunctionNode::FunctionNode(std::string name)
    : DNode(name)
{}

FunctionNode::FunctionNode(const FunctionNode* node)
    : DNode(node), function_name(node->getFunctionName())
{
}

std::string FunctionNode::getFunctionName() const
{
    return function_name;
}

void FunctionNode::setFunctionName(std::string value)
{
    function_name = value;
}

bool FunctionNode::operator==(const DNode &node)const
{
    if(!DNode::operator==(node)) return false;
    if(function_name != node.getDerivedConst<FunctionNode>()->function_name)
        return false;
    return true;
}

bool FunctionNode::operator!=(const DNode &node)const
{
    return (!(operator==(node)));
}

ContainerNode::ContainerNode(std::string name, bool raw)
    : DNode(name), containerData(0)
{
    setNodeType(CONTAINER);
    if(!raw)
    {
        setContainerData(new ContainerSpace);
        containerData->setName(name.c_str());
        new SocketNode(IN, this);
        new SocketNode(OUT, this);
    }
}

ContainerNode::ContainerNode(const ContainerNode* node)
    : DNode(node)
{
    setNodeType(CONTAINER);
    setContainerData(new ContainerSpace(node->getContainerData()));
    foreach(const DSocket *socket, node->getMappedSocketsOnContainer())
        mapOnToIn(CopySocketMapper::getCopy(socket), CopySocketMapper::getCopy(node->getSocketInContainer(socket)));

    setInputs(CopyNodeMapper::getCopy(node->getInputs()));
    setOutputs(CopyNodeMapper::getCopy(node->getOutputs()));
}

ContainerNode::~ContainerNode()
{
    if(containerData) delete containerData;
}

NodeList ContainerNode::getAllInNodes(NodeList nodes)    
{
    nodes = DNode::getAllInNodes(nodes);
    LLsocket *tmp = getOutputs()->getInSocketLlist()->getFirst();
    do{
        if(tmp->socket->getArray())
            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())     
                nodes = socket->getNode()->getAllInNodes(nodes);
        else
            if(tmp->socket->toIn()->getCntdSocket())
                nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodes(nodes);
    }while(tmp = tmp->next);
    return nodes;
}

ConstNodeList ContainerNode::getAllInNodesConst(ConstNodeList nodes)    const
{
    nodes = DNode::getAllInNodesConst(nodes);
    LLsocket *tmp = getOutputs()->getInSocketLlist()->getFirst();
    do{
        if(tmp->socket->getArray())
            foreach(DoutSocket *socket, ((DAInSocket*)tmp->socket)->getLinks())     
                nodes = socket->getNode()->getAllInNodesConst(nodes);
        else
            if(tmp->socket->toIn()->getCntdSocket())
                nodes = tmp->socket->toIn()->getCntdSocket()->getNode()->getAllInNodesConst(nodes);
    }while(tmp = tmp->next);
    return nodes;
}

void ContainerNode::addMappedSocket(DSocket *socket)
{
    DSocket *mapped_socket = 0;
    if(socket->getDir() == IN)
        mapped_socket = new DoutSocket(socket->getName(), socket->getType(), inSocketNode);
    else
        mapped_socket = new DinSocket(socket->getName(), socket->getType(), outSocketNode);
    mapOnToIn(socket, mapped_socket);     
}

void ContainerNode::addtolib()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    QString filename;
    filename.append("nodes/");
    filename.append(getNodeName().c_str());
    filename.append(".node");
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out<<FRG_NODE_HEADER;
    out<<this;
}

void ContainerNode::C_addItems(QList<DNode *> nodes)
{
    foreach(DNode *node, nodes)
        containerData->addNode(node);
}

void ContainerNode::setInputs(SocketNode *inputNode)
{
    inSocketNode = inputNode;
    if(!containerData->getNodes().contains(inputNode))
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
    if(socket->getDir() == IN)
        mapped_socket = new DoutSocket(*socket->toOut(), this);
    else
        mapped_socket = new DinSocket(*socket->toIn(), this);
    mapOnToIn(socket, mapped_socket);
}

void ContainerNode::killSocket(DSocket *socket)
{
    DSocket *contsocket = const_cast<DSocket*>(socket_map.value(socket));
    socket_map.remove(socket);
    removeSocket(contsocket);
}

DSocket *ContainerNode::getSocketInContainer(DSocket *socket)
{
    return const_cast<DSocket*>(socket_map.value(const_cast<DSocket*>(socket)));
}

const DSocket *ContainerNode::getSocketInContainer(const DSocket *socket) const
{
    return socket_map.value(socket);
}

void ContainerNode::mapOnToIn(const DSocket *on, const DSocket *in)    
{
    if(!on &&!in) return;
    socket_map.insert(on, in);
    //DSocket::nameCB(on, in);
    //DSocket::typeCB(on, in);
    //DSocket::nameCB(in, on);
    //DSocket::typeCB(in, on);
}

DSocket *ContainerNode::getSocketOnContainer(DSocket *socket)
{
    return const_cast<DSocket*>(socket_map.key(const_cast<DSocket*>(socket)));
}

const DSocket *ContainerNode::getSocketOnContainer(const DSocket *socket) const
{
    return socket_map.key(socket);
}

QList<const DSocket*> ContainerNode::getMappedSocketsOnContainer() const
{
    return socket_map.keys();
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

ConditionContainerNode::ConditionContainerNode(bool raw)
    : ContainerNode("Condition", raw)
{
    setNodeType(CONDITIONCONTAINER);
    if(!raw)
    {
        new DinSocket("Condition", CONDITION, this);
    }
}

ConditionContainerNode::ConditionContainerNode(const ConditionContainerNode* node)
    : ContainerNode(node)
{
    setNodeType(CONDITIONCONTAINER);
}

ConditionNode::ConditionNode(NType t, bool raw)
{
    setNodeType(t);

    if(!raw)
    {
        new DoutSocket("Output", CONDITION, this);
        if(getNodeType() != AND && getNodeType() != OR)
        {
            new DinSocket("Input", getNodeType() == NOT ? CONDITION : VARIABLE, this);
            if(getNodeType() != NOT)
                new DinSocket("Input", VARIABLE, this);
        }
        switch(getNodeType())
        {
        case GREATERTHAN:
            setNodeName("Greater Than");
            break;
        case SMALLERTHAN:
            setNodeName("Smaller Than");
            break;
        case EQUAL:
            setNodeName("Equal");
            break;
        case NOT:
            setNodeName("Not");
            break;
        case AND:
            setNodeName("And");
            setDynamicSocketsNode(IN, CONDITION);
            break;
        case OR:
            setNodeName("Or");
            setDynamicSocketsNode(IN, CONDITION);
            break;
        default:
            break;
        }
    }
}

ConditionNode::ConditionNode(const ConditionNode* node)
    : DNode(node)
{
}

bool DNode::isValueNode()    const
{
    return getNodeType() == FLOATNODE
        || getNodeType() == STRINGNODE
        || getNodeType() == COLORNODE
        || getNodeType() == VECTORNODE;
}

bool DNode::isConditionNode(const DNode *node)    
{
    return node->getNodeType() == GREATERTHAN
    || node->getNodeType() == SMALLERTHAN
    || node->getNodeType() == NOT
    || node->getNodeType() == EQUAL;
}

ValueNode::ValueNode(std::string name)
    : DNode(name), shaderInput(false)
{
}

ValueNode::ValueNode(const ValueNode* node)
    : DNode(node), shaderInput(node->isShaderInput())
{
}

void ValueNode::setNodeName(std::string name)    
{
    DNode::setNodeName(name);
    if(!getOutSockets().isEmpty())
        getOutSockets().first()->setName(name.c_str());
}

void ValueNode::setShaderInput(bool si)
{
    shaderInput = si;
}

bool ValueNode::isShaderInput() const
{
    return shaderInput;
}

ColorValueNode::ColorValueNode(std::string name, bool raw)
    : ValueNode(name) 
{
    setNodeType(COLORNODE);
    if(!raw)
    {
        new DinSocket("Color", COLOR, this);
        new DoutSocket("Color", COLOR, this);
    }

    setNodeName("Color");
}

ColorValueNode::ColorValueNode(const ColorValueNode* node)
    : ValueNode(node)
{
}

StringValueNode::StringValueNode(std::string name, bool raw)
    : ValueNode(name)
{
    setNodeType(STRINGNODE);
    setNodeName("String");
    if(!raw)
    {
        new DinSocket("String", STRING, this);
        new DoutSocket("String", STRING, this);
    }
}

StringValueNode::StringValueNode(const StringValueNode* node)
    : ValueNode(node)
{
}

FloatValueNode::FloatValueNode(std::string name, bool raw)
    : ValueNode(name)
{
    setNodeType(FLOATNODE);
    setNodeName("Float");
    if(!raw)
    {
        new DinSocket("Float", FLOAT, this);
        new DoutSocket("Float", FLOAT, this);
    }
}

FloatValueNode::FloatValueNode(const FloatValueNode* node)
    : ValueNode(node)
{
}

IntValueNode::IntValueNode(std::string name, bool raw)
    : ValueNode(name)
{
    setNodeType(INTNODE);
    setNodeName("Integer");
    if(!raw){
        new DinSocket("Integer", INTEGER, this);
        new DoutSocket("Integer", INTEGER, this);
    }
}

IntValueNode::IntValueNode(const IntValueNode* node)
    : ValueNode(node)
{
}

BoolValueNode::BoolValueNode(std::string name, bool raw)
    : ValueNode(name)
{
    setNodeType(BOOLNODE);
    setNodeName("Boolean");
    if(!raw){
        new DinSocket("Boolean", CONDITION, this);
        new DoutSocket("Boolean", CONDITION, this);
    }
}

BoolValueNode::BoolValueNode(const BoolValueNode* node)
    : ValueNode(node)
{
}

VectorValueNode::VectorValueNode(std::string name, bool raw)
    :ValueNode(name)
{
    setNodeType(VECTORNODE);
    setNodeName("Vector");
    if(!raw)
    {
        new DinSocket("Vector", VECTOR, this);
        new DoutSocket("Vector", VECTOR, this);
    }
}

VectorValueNode::VectorValueNode(const VectorValueNode* node)
    : ValueNode(node)
{
}

FloatToVectorNode::FloatToVectorNode(bool raw)
    :DNode("Vector")
{
    setNodeType(FLOATTOVECTOR);
    if(!raw)
    {
        new DinSocket("X", FLOAT, this);
        new DinSocket("Y", FLOAT, this);
        new DinSocket("Z", FLOAT, this);
        new DoutSocket("Vector", VECTOR, this);
    }
}

FloatToVectorNode::FloatToVectorNode(const FloatToVectorNode* node)
    : DNode(node)
{
}

SocketNode::SocketNode(socket_dir dir, ContainerNode *contnode, bool raw)
    : container(0)
{
    if (dir == IN)
    {
        setNodeType(INSOCKETS);
        if(!raw && contnode)setInSocketNode(contnode);
    }
    else
    {
        setNodeType(OUTSOCKETS);
        if(!raw && contnode)setOutSocketNode(contnode);
    }
}

SocketNode::SocketNode(const SocketNode* node)
    : DNode(node), container(CopyNodeMapper::getCopy(static_cast<DNode*>(node->getContainer()))->getDerived<ContainerNode>())
{
}

void SocketNode::setInSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(OUT);
    setNodeName("Input");
    contnode->setInputs(this);
}

void SocketNode::setOutSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(IN);
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
	if(getLastSocket()->getDir() == IN)
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

LoopSocketNode::LoopSocketNode(socket_dir dir, LoopNode *contnode, bool raw)
    :SocketNode(dir, contnode, true), partner(0)
{
    if (dir == IN) {
        setNodeType(LOOPINSOCKETS);
        if(!raw && contnode) {
            //setDynamicSocketsNode(OUT);
            setNodeName("Looped Sockets");
        }
    }
    else {
        setNodeType(LOOPOUTSOCKETS);
        if(!raw && contnode)setOutSocketNode(contnode);
    }
}

LoopSocketNode::LoopSocketNode(const LoopSocketNode* node)
    : SocketNode(node), partner(0)
{
    foreach(DSocket *original, node->getLoopedSockets())
        loopSocketMap.insert(const_cast<DSocket*>(CopySocketMapper::getCopy(original)),
                             const_cast<DSocket*>(CopySocketMapper::getCopy(node->getPartnerSocket(original))));        
}

void LoopSocketNode::dec_var_socket(DSocket *socket)
{
    SocketNode::dec_var_socket(socket);
    if(partner != 0)
    {
        LoopSocketNode *tmpp = partner;
        partner->setPartner(0);
        deletePartnerSocket(socket);
        partner->setPartner(tmpp);
    }
}

QList<DSocket*> LoopSocketNode::getLoopedSockets() const
{
    return loopSocketMap.keys();
}

qint16 LoopSocketNode::getLoopedSocketsCount() const
{
    return loopSocketMap.size();
}

void LoopSocketNode::createPartnerSocket(DSocket *socket)
{
	DSocket *partnerSocket;
	DSocket *newsocket;
	if(socket->getDir() == IN)
    {
        partnerSocket = new DoutSocket(socket->getName(), socket->getType(), partner);
		newsocket = new DinSocket(socket->getName(), socket->getType(), getContainer());
        getContainer()->mapOnToIn(newsocket, partnerSocket);
    }
	else
		partnerSocket = new DinSocket(socket->getName(), socket->getType(), partner);
    loopSocketMap.insert(socket, partnerSocket);
    //partner->mapPartner(partnerSocket, socket);
}

void LoopSocketNode::deletePartnerSocket(DSocket *socket)
{
    DinSocket *partnerSocket = getPartnerSocket(socket)->toIn();
    partner->removeSocket(partnerSocket);
    loopSocketMap.remove(socket);
}

void LoopSocketNode::mapPartner(DSocket* here, DSocket *partner)    
{
   loopSocketMap.insert(here, partner); 
}

void LoopSocketNode::setPartner(LoopSocketNode *p)
{
    partner = p;
}

DSocket *LoopSocketNode::getPartnerSocket(const DSocket *socket) const
{
    if(loopSocketMap.keys().contains(const_cast<DSocket*>(socket)))
        return loopSocketMap.value(const_cast<DSocket*>(socket));
    else return 0;
}

void LoopSocketNode::inc_var_socket()
{
    SocketNode::inc_var_socket();
    if(getNodeType() == LOOPOUTSOCKETS)
        createPartnerSocket(getLastSocket());
}

LoopNode::LoopNode(std::string name, bool raw)
    : ContainerNode(name, true)
{
    if(!raw)
    {
        LoopSocketNode *loutNode, *linNode, *loopNode;
        setContainerData(new ContainerSpace);
        getContainerData()->setName(name.c_str());

        loopNode = new LoopSocketNode(IN, this);
        setLoopedSockets(loopNode);
        linNode = new LoopSocketNode(IN, this);
        setInputs(linNode);
        linNode->setDynamicSocketsNode(OUT);
        loutNode = new LoopSocketNode(OUT, this);

        loopNode->setPartner(loutNode);
        loutNode->setPartner(loopNode);
    }
}

LoopNode::LoopNode(const LoopNode* node)
    : ContainerNode(node)
{
}

LoopSocketNode* LoopNode::getLoopedInputs()    
{
    return loopSockets; 
}

void LoopNode::setLoopedSockets(LoopSocketNode *node)    
{
    loopSockets = node;
    if(!getContainerData()->getNodes().contains(node))
        getContainerData()->addNode(node);
    node->connectToContainer(this);
}

bool LoopNode::isLoopNode(DNode *node)
{
    return (node->getNodeType() == FOR
            ||node->getNodeType() == WHILE
            ||node->getNodeType() == ILLUMINANCE
            ||node->getNodeType() == ILLUMINATE
            ||node->getNodeType() == SOLAR
            ||node->getNodeType() == GATHER);
}
WhileNode::WhileNode(bool raw)
    : LoopNode("While", raw)
{
    setNodeType(WHILE);
    if(!raw)
    {
        new DinSocket("Condition", CONDITION, getOutputs());
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

WhileNode::WhileNode(const WhileNode* node)
    : LoopNode(node)
{
}

ForNode::ForNode(bool raw)
    : LoopNode("For", raw)
{
    setNodeType(FOR);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Start", INTEGER, this));
        addMappedSocket(new DinSocket("End", INTEGER, this));
        addMappedSocket(new DinSocket("Step", INTEGER, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

ForNode::ForNode(const ForNode* node)
    : LoopNode(node)
{
}

ForeachNode::ForeachNode(bool raw)
    : ContainerNode("Foreach", raw)
{
    setNodeType(FOREACHNODE);
    if(!raw)
    {
        new DinSocket("Array", VARIABLE, this);
        new DoutSocket("Array", VARIABLE, this);
        new DoutSocket("Index", INTEGER, getInputs());
        new DinSocket("Value", VARIABLE, getOutputs());
        DSocketList *inputSocketList = getInputs()->getDerived<SocketNode>()->getOutSocketLlist();
        DSocketList *outSocketList = getOutputs()->getDerived<SocketNode>()->getInSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
        outSocketList->move(0, outSocketList->len() - 1);
    }
}

ForeachNode::ForeachNode(const ForeachNode* node)
    : ContainerNode(node)
{
}

IlluminanceNode::IlluminanceNode(bool raw)
    : LoopNode("Illuminance", raw)
{
    setNodeType(ILLUMINANCE);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Category", STRING, this));
        addMappedSocket(new DinSocket("Point", POINT, this));
        addMappedSocket(new DinSocket("Direction", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        addMappedSocket(new DinSocket("Message Passing", STRING, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

IlluminanceNode::IlluminanceNode(const IlluminanceNode* node)
    : LoopNode(node)
{
}

IlluminateNode::IlluminateNode(bool raw)
    : LoopNode("Illuminate", raw)
{
    setNodeType(ILLUMINATE);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Point", POINT, this));
        addMappedSocket(new DinSocket("Direction", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

IlluminateNode::IlluminateNode(const IlluminateNode* node)
    : LoopNode(node)
{
}

GatherNode::GatherNode(bool raw)
    : LoopNode("Gather", raw)
{
    setNodeType(GATHER);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Category", STRING, this));
        addMappedSocket(new DinSocket("Point", POINT, this));
        addMappedSocket(new DinSocket("Direction", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        addMappedSocket(new DinSocket("Message Passing", STRING, this));
        addMappedSocket(new DinSocket("Samples", FLOAT, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
    getInSocketLlist()->move(0, getInSocketLlist()->len() - 1);
}

GatherNode::GatherNode(const GatherNode* node)
    : LoopNode(node)
{
}

SolarNode::SolarNode(bool raw)
    : LoopNode("Solar", raw)
{
    setNodeType(SOLAR);
    if(!raw)
    {
        addMappedSocket(new DinSocket("Axis", VECTOR, this));
        addMappedSocket(new DinSocket("Angle", FLOAT, this));
        DSocketList *inputSocketList = getInputs()->getDerived<LoopSocketNode>()->getOutSocketLlist();
        inputSocketList->move(0, inputSocketList->len() - 1);
    }
}

SolarNode::SolarNode(const SolarNode* node)
    : LoopNode(node)
{
}

GetArrayNode::GetArrayNode(bool raw)
    : DNode("GetArray")
{
    setNodeType(GETARRAY);
    if(!raw)
    {
        DinSocket *arr = new DinSocket("Array", VARIABLE, this);
        new DinSocket("Index", FLOAT, this);
        DoutSocket *val = new DoutSocket("value", VARIABLE, this);
    }
}

GetArrayNode::GetArrayNode(const GetArrayNode* node)
    : DNode(node)
{
    DinSocket *arr = getInSockets().first();
    DoutSocket *val = getOutSockets().first();
}

SetArrayNode::SetArrayNode(bool raw)
    : DNode("SetArray")
{
    setNodeType(SETARRAY);
    if(!raw){
        DinSocket *inarr = new DinSocket("Array", VARIABLE, this);
        DinSocket *val = new DinSocket("value", VARIABLE, this);
        new DinSocket("Index", INTEGER, this);
        DoutSocket *arr = new DoutSocket("Array", VARIABLE, this);
    }
}

SetArrayNode::SetArrayNode(const SetArrayNode* node)
    : DNode(node)
{
    DinSocket *val = getInSockets().first();
    DoutSocket *arr = getOutSockets().first();
}

ComposeArrayNode::ComposeArrayNode(bool raw)
    : DNode("Compose Array")
{
    setNodeType(COMPOSEARRAY);
    if(!raw){
        DoutSocket *out = new DoutSocket("", VARIABLE, this);
        new DAInSocket("+", VARIABLE, this);
        //setDynamicSocketsNode(IN);
    }
}

ComposeArrayNode::ComposeArrayNode(const ComposeArrayNode* node)
    : DNode(node)
{
}

VarNameNode::VarNameNode(bool raw)
    : DNode("Var Name")
{
    setNodeType(VARNAME);
    if(!raw){
        DinSocket *in = new DinSocket("def", VARIABLE, this);
        DoutSocket *out =  new DoutSocket("variable", VARIABLE, this);
    }
}

VarNameNode::VarNameNode(const VarNameNode* node)
    : DNode(node)
{
    DinSocket *in = getInSockets().first();
    DoutSocket *out = getOutSockets().first();
}

WriteFileNode::WriteFileNode(bool raw)
    : DNode("Write File")
{
    setNodeType(WRITEFILE);
    if(!raw){
        DinSocket *fileName = new DinSocket("Filename", STRING, this);
        new DinSocket("File Content", STRING, this);
        new DoutSocket("Filename", STRING, this);
    }
}

WriteFileNode::WriteFileNode(const WriteFileNode* node)
    : DNode(node)
{
}

ReadFileNode::ReadFileNode(bool raw)
    : DNode("Read File")
{
    setNodeType(READFILE);
    if(!raw){
        DinSocket *fileName = new DinSocket("Filename", STRING, this);
        new DoutSocket("File Content", STRING, this);
    }
}

ReadFileNode::ReadFileNode(const ReadFileNode* node)
    : DNode(node)
{
}

ProcessNode::ProcessNode(bool raw)
    : DNode("External Process")
{
    setNodeType(PROCESS);
    if(!raw){
        DinSocket *executable = new DinSocket("Executable", STRING, this);
        new DinSocket("Parameters", STRING, this);
    }
}

ProcessNode::ProcessNode(const ProcessNode* node)
    : DNode(node)
{
}

LoadImageNode::LoadImageNode(bool raw)
    : DNode("Load Image")
{
    setNodeType(LOADIMAGE);
    if(!raw){
        DinSocket *imageFile = new DinSocket("Filename", STRING, this);
        new DoutSocket("Image", IMAGE, this);
    }
}

LoadImageNode::LoadImageNode(const LoadImageNode* node)
    : DNode(node)
{
}

SaveImageNode::SaveImageNode(bool raw)
    : DNode("Save Image")
{
    setNodeType(SAVEIMAGE);
    if(!raw){
        DinSocket *image = new DinSocket("Image", IMAGE, this);
        DinSocket *imageFile = new DinSocket("Filename", STRING, this);
    }
}

SaveImageNode::SaveImageNode(const SaveImageNode* node)
    : DNode(node)
{
}
