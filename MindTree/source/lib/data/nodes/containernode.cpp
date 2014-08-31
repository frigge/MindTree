#include "data/dnspace.h"
#include "containernode.h"

using namespace MindTree;

ContainerNode::ContainerNode(std::string name, bool raw)
    : DNode(name), containerData(0)
{
    setType("CONTAINER");

    setContainerData(new ContainerSpace);
    containerData->setName(name);
    new SocketNode(DSocket::IN, this, raw);
    new SocketNode(DSocket::OUT, this, raw);
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

void ContainerNode::setSpace(DNSpace *space)
{
    DNode::setSpace(space);
    if(containerData) containerData->setParentSpace(space);
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
    return nullptr;
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

SocketNode::SocketNode(DSocket::SocketDir dir, ContainerNode *contnode, bool raw)
    : container(0)
{
    if (dir == DSocket::IN)
    {
        setType("INSOCKETS");
        if(!raw && contnode)setInSocketNode(contnode);
        contnode->setInputs(this);
    }
    else
    {
        setType("OUTSOCKETS");
        if(!raw && contnode)setOutSocketNode(contnode);
        contnode->setOutputs(this);
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
}

void SocketNode::setOutSocketNode(ContainerNode *contnode)
{
    setDynamicSocketsNode(DSocket::IN);
    setNodeName("Output");
}

void SocketNode::connectToContainer(ContainerNode *contnode)
{
    container = contnode;
}

ContainerNode* SocketNode::getContainer() const
{
    return container;
}

void SocketNode::incVarSocket()
{
    DNode::incVarSocket();
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

void SocketNode::decVarSocket(DSocket *socket)
{
    if(container)container->removeSocket(container->getSocketOnContainer(socket));
    DNode::decVarSocket(socket);
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
//void LoopSocketNode::decVarSocket(DSocket *socket)
//{
//    SocketNode::decVarSocket(socket);
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
//void LoopSocketNode::incVarSocket()
//{
//    SocketNode::incVarSocket();
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
