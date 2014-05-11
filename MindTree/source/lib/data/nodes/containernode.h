#ifndef CONTAINERNODE_H
#define CONTAINERNODE_H

#include "data_node.h"

namespace MindTree {
class ContainerSpace;
class SocketNode;

class ContainerNode : public DNode
{
public:
    ContainerNode(std::string name="", bool raw=false);
    ContainerNode(const ContainerNode &node);
    ~ContainerNode();

    DSocket *getSocketOnContainer(DSocket *socket);
    const DSocket *getSocketOnContainer(const DSocket *socket) const;
    DSocket *getSocketInContainer(DSocket *socket);
    const DSocket *getSocketInContainer(const DSocket *socket) const;
    std::vector<const DSocket*> getMappedSocketsOnContainer() const;
    void mapOnToIn(const DSocket *on, const DSocket *in);
    int getSocketMapSize() const;

    NodeList getAllInNodes();
    ConstNodeList getAllInNodesConst() const;

    ContainerSpace* getContainerData() const;
    void setContainerData(ContainerSpace* value);
    void addItems(NodeList nodes);

    void addMappedSocket(DSocket *socket);

    void setInputs(SocketNode *inputNode);
    void setInputs(DNode *inputNode);
    void setOutputs(SocketNode *outputNode);
    void setOutputs(DNode *outputNode);
    SocketNode* getInputs() const;
    SocketNode* getOutputs() const;

    void setNodeName(std::string name);
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;

    void newSocket(DSocket *socket);
    void killSocket(DSocket *socket);
    void addtolib();

private:
    SocketNode *inSocketNode, *outSocketNode;
    ContainerSpace *containerData;
    std::unordered_map<const DSocket*, const DSocket*> socket_map;
};

class SocketNode : public DNode
{
public:
    SocketNode(DSocket::SocketDir dir, ContainerNode *contnode, bool raw=false);
    SocketNode(const SocketNode &node);

    void setInSocketNode(ContainerNode *contnode);
    virtual void setOutSocketNode(ContainerNode *contnode);

    virtual void incVarSocket();
    virtual void decVarSocket(DSocket *socket);

    void connectToContainer(ContainerNode*);
    ContainerNode* getContainer() const;


private:
    ContainerNode *container;
};

class LoopSocketNode;
class LoopNode : public ContainerNode
{
public:
    LoopNode(std::string name="", bool raw=false);
    LoopNode(const LoopNode* node);
    static bool isLoopNode(DNode *);
    void setLoopedSockets(LoopSocketNode *node);
    LoopSocketNode* getLoopedInputs();

private:
    LoopSocketNode *loopSockets;
};

class LoopSocketNode : public SocketNode
{
public:
    LoopSocketNode(DSocket::SocketDir dir, LoopNode *contnode, bool raw=false);
    LoopSocketNode(const LoopSocketNode* node);

    virtual void decVarSocket(DSocket *socket);
    void createPartnerSocket(DSocket *);
    void deletePartnerSocket(DSocket *);
    void mapPartner(DSocket* here, DSocket *partner);

    void setPartner(LoopSocketNode* p);
    DSocket *getPartnerSocket(const DSocket *) const;
    std::vector<DSocket*> getLoopedSockets() const;
    uint getLoopedSocketsCount() const;

    virtual void incVarSocket();

private:
    QHash<DSocket*, DSocket*> loopSocketMap;
    LoopSocketNode *partner;
};

class ForNode : public LoopNode
{
public:
    ForNode(bool raw=false);
    ForNode(const ForNode* node);
};

class ForeachNode : public ContainerNode
{
public:
    ForeachNode(bool raw=false);
    ForeachNode(const ForeachNode* node);
};

class WhileNode : public LoopNode
{
public:
    WhileNode(bool raw=false);
    WhileNode(const WhileNode* node);
};
}
#endif
