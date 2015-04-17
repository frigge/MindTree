#include "data/dnspace.h"
#include "containernode.h"

using namespace MindTree;

ContainerNode::ContainerNode(std::string name, bool raw)
    : DNode(name), 
    containerData(nullptr), 
    inSocketNode(nullptr), 
    outSocketNode(nullptr)
{
    setBuildInType(CONTAINER);
    setType("CONTAINER");

    setContainerData(new ContainerSpace);
    containerData->setName(name);
    if(!raw) {
        auto in = std::make_shared<SocketNode>(DSocket::IN, this, false);
        setInSockets(in.get());
        auto out = std::make_shared<SocketNode>(DSocket::OUT, this, false);
        setOutSockets(out.get());

        containerData->addNode(in);
        containerData->addNode(out);
    }
}

ContainerNode::ContainerNode(const ContainerNode &node)
    : DNode(node)
{
    setBuildInType(CONTAINER);
    setType("CONTAINER");
    setContainerData(new ContainerSpace(*node.getContainerData()));

    for(const DSocket *socket : node.getMappedSocketsOnContainer())
        mapOnToIn(CopySocketMapper::getCopy(socket),
                  CopySocketMapper::getCopy(node.getSocketInContainer(socket)));

    setInSockets(dynamic_cast<SocketNode*>(CopyNodeMapper::getCopy(node.getInputs())));
    setOutSockets(dynamic_cast<SocketNode*>(CopyNodeMapper::getCopy(node.getOutputs())));
}

ContainerNode::~ContainerNode()
{
    if(containerData) {
        delete containerData;
        containerData = nullptr;
    }
}

void ContainerNode::setInSockets(SocketNode *node)
{
    if(inSocketNode) {
        containerData->removeNode(inSocketNode);
    }
    inSocketNode = node;
}

void ContainerNode::setOutSockets(SocketNode *node)
{
    if(outSocketNode) {
        containerData->removeNode(outSocketNode);
    }
    outSocketNode = node;
}

void ContainerNode::setSpace(DNSpace *space)
{
    DNode::setSpace(space);
    if(containerData) containerData->setParentSpace(space);
}

std::vector<DNode*> ContainerNode::getAllInNodes()
{
    std::vector<DNode*> nodes = DNode::getAllInNodes();
    for(auto socket : getOutputs()->getInSockets()) {
        DoutSocket *cntd = nullptr;
        if((cntd = socket->toIn()->getCntdSocket())) {
            std::vector<DNode*> morenodes = cntd->getNode()->getAllInNodes();
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
    DSocket *mapped_socket = nullptr;
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

SocketNode *ContainerNode::getInputs() const
{
    return inSocketNode;
}

SocketNode *ContainerNode::getOutputs() const
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

void ContainerNode::mapOnToIn(DSocket *on, DSocket *in)
{
    if(!on &&!in) return;
    socket_map.insert({on, in});
    on->listenToChange(in);
    in->listenToChange(on);
    on->listenToLinkedType();
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
    return nullptr;
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

void ContainerNode::setName(std::string name)
{
    DNode::setName(name);
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
    : container(contnode)
{
    if (dir == DSocket::IN)
    {
        setType("INSOCKETS");
        setName("Input");
    }
    else
    {
        setType("OUTSOCKETS");
        setName("Output");
    }
    if(!raw && contnode) {
        setDynamicSocketsNode(dir == DSocket::OUT ? DSocket::IN : DSocket::OUT);
        getVarSocket()->listenToLinkedType();
    }
}

SocketNode::SocketNode(const SocketNode &node)
    : DNode(node), 
    container(CopyNodeMapper::getCopy(static_cast<DNode*>(node.getContainer()))
              ->getDerived<ContainerNode>())
{
}

ContainerNode* SocketNode::getContainer() const
{
    return container;
}

void SocketNode::incVarSocket()
{
    DNode::incVarSocket();
	DSocket *newsocket;
	if(getLastSocket()->getDir() == DSocket::IN) {
		newsocket = new DoutSocket(getLastSocket()->getName(), getLastSocket()->getType(), container);
    }
	else {
		newsocket = new DinSocket(getLastSocket()->getName(), getLastSocket()->getType(), container);
    }
    container->mapOnToIn(newsocket, getLastSocket());
    getVarSocket()->listenToLinkedType();
}

void SocketNode::decVarSocket(DSocket *socket)
{
    if(container)container->removeSocket(container->getSocketOnContainer(socket));
    DNode::decVarSocket(socket);
}

LoopSocketNode::LoopSocketNode(DSocket::SocketDir dir, LoopNode *contnode, bool raw)
    : SocketNode(dir, contnode, true), partner(nullptr)
{
    if (dir == DSocket::IN) {
        setType("LOOPINSOCKETS");
        setName("Looped Sockets");

        if(!raw && contnode) {
            setDynamicSocketsNode(DSocket::OUT);
            getVarSocket()->listenToLinkedType();
        }
    }
    else {
        setType("LOOPOUTSOCKETS");
        setName("Outputs");
    }
}

LoopSocketNode::LoopSocketNode(const LoopSocketNode& node)
    : SocketNode(node), partner(nullptr)
{
    for(auto pair : node.loopSocketMap) {
        auto *original = const_cast<DSocket*>(CopySocketMapper::getCopy(pair.first));
        auto *partner = const_cast<DSocket*>(CopySocketMapper::getCopy(pair.second));
        auto newPair = std::make_pair(original, partner);
        loopSocketMap.insert(newPair);
    }
}

void LoopSocketNode::decVarSocket(DSocket *socket)
{
    SocketNode::decVarSocket(socket);
    if(partner)
    {
        LoopSocketNode *tmpp = partner;
        partner->setPartner(nullptr);
        deletePartnerSocket(socket);
        partner->setPartner(tmpp);
    }
}

std::vector<DSocket*> LoopSocketNode::getLoopedSockets() const
{
    std::vector<DSocket*> keys;
    for (auto p : loopSocketMap)
        keys.push_back(p.first);

    return keys;
}

uint LoopSocketNode::getLoopedSocketsCount() const
{
    return loopSocketMap.size();
}

void LoopSocketNode::createPartnerSocket(DSocket *socket)
{
    //only the input socketnode creates a partnersocket on the output
	DSocket *partnerSocket;
	DSocket *newsocket;
    partnerSocket = new DinSocket(socket->getName(), socket->getType(), partner);
    newsocket = new DoutSocket(socket->getName(), socket->getType(), getContainer());

    getContainer()->mapOnToIn(newsocket, partnerSocket);
    loopSocketMap.insert({socket, partnerSocket});
    partner->mapPartner(partnerSocket, socket);

    partnerSocket->listenToTypeChange(socket);
    partnerSocket->listenToLinkedType();
    newsocket->listenToLinkedType();
    socket->listenToTypeChange(partnerSocket);
}

void LoopSocketNode::deletePartnerSocket(DSocket *socket)
{
    DinSocket *partnerSocket = getPartnerSocket(socket)->toIn();
    partner->removeSocket(partnerSocket);
    loopSocketMap.erase(socket);
}

void LoopSocketNode::mapPartner(DSocket* here, DSocket *partner)    
{
   loopSocketMap.insert({here, partner});
}

void LoopSocketNode::setPartner(LoopSocketNode *p)
{
    partner = p;
}

DSocket *LoopSocketNode::getPartnerSocket(const DSocket *socket) const
{
    if(loopSocketMap.find(const_cast<DSocket*>(socket)) != end(loopSocketMap))
        return loopSocketMap.at(const_cast<DSocket*>(socket));
    else 
        return nullptr;
}

void LoopSocketNode::incVarSocket()
{
    SocketNode::incVarSocket();
    createPartnerSocket(getLastSocket());
}

LoopNode::LoopNode(std::string name, bool raw)
    : ContainerNode(name, true)
{
    if(!raw)
    {
        setContainerData(new ContainerSpace);
        getContainerData()->setName(name.c_str());

        auto in = std::make_shared<SocketNode>(DSocket::IN, this);
        in->setType("LOOPINPUTS");
        in->setName("Static Inputs");
        inputNode = in.get();

        auto loop = std::make_shared<LoopSocketNode>(DSocket::IN, this);
        looped = loop.get();
        auto lout = std::make_shared<LoopSocketNode>(DSocket::OUT, this);
        loopOutputs = lout.get();
        setInSockets(inputNode);
        getContainerData()->addNode(in);
        getContainerData()->addNode(loop);
        getContainerData()->addNode(lout);
        setOutSockets(loopOutputs);

        looped->setPartner(loopOutputs);
        loopOutputs->setPartner(looped);
    }
}

LoopNode::LoopNode(const LoopNode& node)
    : ContainerNode(node)
{
}

LoopSocketNode* LoopNode::getLoopedInputs() const
{
    return looped; 
}

WhileNode::WhileNode(bool raw)
    : LoopNode("While", raw)
{
    setType("WHILE");
    if(!raw)
    {
        new DinSocket("Condition", "BOOLEAN", getOutputs());
    }
}

WhileNode::WhileNode(const WhileNode& node)
    : LoopNode(node)
{
}

ForNode::ForNode(bool raw)
    : LoopNode("For", raw)
{
    setType("FOR");
    if(!raw)
    {
        addMappedSocket(new DinSocket("Start", "INTEGER", this));
        addMappedSocket(new DinSocket("End", "INTEGER", this));
        addMappedSocket(new DinSocket("Step", "INTEGER", this));
    }
}

ForNode::ForNode(const ForNode& node)
    : LoopNode(node)
{
}

ForeachNode::ForeachNode(bool raw)
    : LoopNode("Foreach", raw)
{
    setType("FOREACH");
    if(!raw)
    {
        //undo dynamic sockets on inputs
        auto *var = getInputs()->getVarSocket();
        getInputs()->removeSocket(var);

        var = getContainerData()->getNodes()[1]->getVarSocket();
        getContainerData()->getNodes()[1]->removeSocket(var);

        setDynamicSocketsNode(DSocket::IN);
    }
}

ForeachNode::ForeachNode(const ForeachNode& node)
    : LoopNode(node)
{
}

void ForeachNode::incVarSocket()
{
    DNode::incVarSocket();

    auto nodes = getContainerData()->getNodes();

    auto t = getLastSocket()->getType();
    if(t.toStr().find("LIST:") != std::string::npos) {
        auto singleType = t.toStr().substr(t.toStr().find(":") + 1);
        new DoutSocket(getLastSocket()->getName(), singleType, nodes[1].get());
        new DoutSocket(getLastSocket()->getName(), t, this);
        new DinSocket(getLastSocket()->getName(), singleType, nodes[2].get());
    }
    else {
        new DoutSocket(getLastSocket()->getName(), t, nodes[0].get());
    }
}
