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

#include "QPointF"
#include "QColor"

#include "data/nodes/data_node_socket.h"
#include "data/callbacks.h"
#include "data/python/pyexposable.h"
#include "data/nodes/nodetype.h"

#include "functional"
#include "memory"

class VNode;
class VContainerNode;
class SourceDock;

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
    static QHash<unsigned short, DNode_ptr>loadIDMapper;
};

class CopyNodeMapper
{
public:
    static void setNodePair(DNode *original, DNode *copy);
    static DNode * getCopy(DNode *original);

private:
    static QHash<DNode*, DNode*> nodeMap;
};

class DNSpace;
class ContainerNode;

typedef QList<DNode*> NodeList;
typedef QList<const DNode*> ConstNodeList;


class DNode : public QObject, public PyExposable
{
Q_OBJECT
public:
    DNode(std::string name="");
    DNode(const DNode* node);
    DNode(const DNode& node);
    static QList<DNode*> copy(QList<DNode*>nodes);
    virtual ~DNode();

    bool getSelected();
    void setSelected(bool value);

    NodeType getType()const;
    void setType(std::string value);
    virtual DNode* clone();

    QPointF getPos()const;
    void setPos(QPointF value);

    template<class C>
    const C* getDerivedConst() const;

    template<class C>
    C* getDerived();

    virtual NodeList getAllInNodes(NodeList nodes=NodeList());
    virtual ConstNodeList getAllInNodesConst(ConstNodeList nodes=ConstNodeList()) const;

    virtual void setNodeName(std::string name);
    std::string getNodeName() const;
    void setNodeType(NType t);
    void setNodeType(NodeType t);
    virtual void addSocket(DSocket *socket);
    QStringList getSocketNames();
    void setSocketIDName(DSocket *socket);
    DSocket* getSocketByIDName(std::string idname);
    void removeSocket(DSocket *socket);
    virtual void dec_var_socket(DSocket *socket);
    virtual void inc_var_socket();
    void setDynamicSocketsNode(socket_dir dir, socket_type t=VARIABLE);
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
    NType getNodeType() const;
    DSocket* getVarSocket() const;
    void setVarSocket(const DSocket* value);
    DSocket* getLastSocket() const;
    void setLastSocket(const DSocket* value);
    int getVarcnt() const;
    void setVarcnt(int value);
    DNSpace* getSpace() const;
    virtual void setSpace(DNSpace* value);

    //void regAddSocketCB(Callback *cb);
    //void remAddSocketCB(Callback *cb);
    //void unblockCB();
    //void blockCB();
    //void unblockRegCB();
    //void blockRegCB();

    static DNode_ptr newNode(std::string name, NType t, int insize, int outsize);

    static bool isInput(const DNode *node);
    //static bool isMathNode(const DNode *node);
    static bool isConditionNode(const DNode *node);
    bool isValueNode()const;
    
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;

    bool isGhost();

    DNode *createFuncNode(std::string filepath);
    static DNode *dropNode(std::string filepath);
    //static ContainerNode *buildContainerNode(QList<DNode*>nodes);
    //static void unpackContainerNode(DNode *node);

    Property getProperty(std::string name);
    const Property getProperty(std::string name)const;
    PropertyMap getProperties()const;
    void setProperty(Property value);
    Property operator[](std::string name);
    void rmProperty(std::string name);

signals:
    void deleted(DNode*);

private:
    static std::vector<std::function<DNode_ptr()>> newNodeFactory;
    bool selected;
    DNSpace *space;
    DSocket *varsocket;
    DSocket *lastsocket;
    bool ghost;
    bool blockCBregister;
    int varcnt;
    unsigned short ID;
    static unsigned short count;
    std::string nodeName;
    mutable DSocketList outSockets;
    mutable DSocketList inSockets;
    NodeType type;
    CallbackList addSocketCallbacks;
    QPointF pos;
    mutable PropertyMap properties;
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

QDataStream &operator<<(QDataStream &stream, DNode *node);
QDataStream &operator>>(QDataStream &stream, DNode_ptr *node);

class FunctionNode : public DNode
{
public:
    FunctionNode(std::string name="");
    FunctionNode(const FunctionNode* node);
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;
    std::string getFunctionName() const;
    void setFunctionName(std::string value);

private:
    std::string function_name;
};

class ContainerSpace;
class SocketNode;

class ContainerNode : public DNode
{
public:
    ContainerNode(std::string name="", bool raw=false);
    ContainerNode(const ContainerNode* node);
    ~ContainerNode();

    DSocket *getSocketOnContainer(DSocket *socket);
    const DSocket *getSocketOnContainer(const DSocket *socket) const;
    DSocket *getSocketInContainer(DSocket *socket);
    const DSocket *getSocketInContainer(const DSocket *socket) const;
    QList<const DSocket*> getMappedSocketsOnContainer() const;
    void mapOnToIn(const DSocket *on, const DSocket *in);
    int getSocketMapSize() const;

    NodeList getAllInNodes(NodeList nodes);
    ConstNodeList getAllInNodesConst(ConstNodeList nodes)const;

    ContainerSpace* getContainerData() const;
    void setContainerData(ContainerSpace* value);
    void C_addItems(QList<DNode*> nodes);

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
    QHash<const DSocket*, const DSocket*> socket_map;
};


class ConditionContainerNode : public ContainerNode
{
public:
    ConditionContainerNode(bool raw=false);
    ConditionContainerNode(const ConditionContainerNode* node);
};

class SocketNode : public DNode
{
public:
    SocketNode(socket_dir dir, ContainerNode *contnode, bool raw=false);
    SocketNode(const SocketNode* node);

    void setInSocketNode(ContainerNode *contnode);
    virtual void setOutSocketNode(ContainerNode *contnode);

    virtual void inc_var_socket();
    virtual void dec_var_socket(DSocket *socket);

    void connectToContainer(ContainerNode*);
    ContainerNode* getContainer() const;


private:
    ContainerNode *container;
};

class LoopNode;
class LoopSocketNode : public SocketNode
{
public:
    LoopSocketNode(socket_dir dir, LoopNode *contnode, bool raw=false);
    LoopSocketNode(const LoopSocketNode* node);

    virtual void dec_var_socket(DSocket *socket);
    void createPartnerSocket(DSocket *);
    void deletePartnerSocket(DSocket *);
    void mapPartner(DSocket* here, DSocket *partner);

    void setPartner(LoopSocketNode* p);
    DSocket *getPartnerSocket(const DSocket *) const;
    QList<DSocket*> getLoopedSockets() const;
    qint16 getLoopedSocketsCount() const;

    virtual void inc_var_socket();

private:
    QHash<DSocket*, DSocket*> loopSocketMap;
    LoopSocketNode *partner;
};

class ConditionNode : public DNode
{
public:
    ConditionNode(NType t, bool raw=false);
    ConditionNode(const ConditionNode *node);
};

class ValueNode : public DNode
{
public:
    ValueNode(std::string name);
    ValueNode(const ValueNode* node);
    bool isShaderInput() const;
    void setShaderInput(bool);

    void setNodeName(std::string name);

protected:
    //virtual void getValue() = 0;

private:
    bool shaderInput;
};

class ColorValueNode : public ValueNode
{
public:
    ColorValueNode(std::string name="Color", bool raw=false);
    ColorValueNode(const ColorValueNode* node);
    //virtual bool operator==(const DNode &node)const;
    //virtual bool operator!=(const DNode &node)const;

    //void setValue(QColor);
    //QColor getValue() const;

//protected:
    //void updateColorLabel();

//private:
    //QColor colorvalue;
};

class StringValueNode : public ValueNode
{
public:
    StringValueNode(std::string name="String", bool raw=false);
    StringValueNode(const StringValueNode* node);
};

class FloatValueNode : public ValueNode
{
public:
    FloatValueNode(std::string name="Float", bool raw=false);
    FloatValueNode(const FloatValueNode* node);
};

class IntValueNode : public ValueNode
{
public:
    IntValueNode(std::string name="Integer", bool raw=false);
    IntValueNode(const IntValueNode* node);
};

class BoolValueNode : public ValueNode
{
public:
    BoolValueNode(std::string name="Boolean", bool raw=false);
    BoolValueNode(const BoolValueNode* node);
};

class VectorValueNode : public ValueNode
{
public:
    VectorValueNode(std::string name="Vector", bool raw=false);
    VectorValueNode(const VectorValueNode* node);
    //Vector getValue() const;
    //void setValue(Vector newvalue);
    //virtual bool operator==(const DNode &node)const;
    //virtual bool operator!=(const DNode &node)const;

//private:
    //Vector vectorvalue;
};

class FloatToVectorNode : public DNode
{
public:
    FloatToVectorNode(bool raw=false);
    FloatToVectorNode(const FloatToVectorNode* node);
};

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

class GatherNode : public LoopNode
{
public:
    GatherNode(bool raw=false);
    GatherNode(const GatherNode* node);
};

class IlluminanceNode : public  LoopNode
{
public:
    IlluminanceNode(bool raw=false);
    IlluminanceNode(const IlluminanceNode* node);
};

class IlluminateNode : public LoopNode
{
public:
    IlluminateNode(bool raw=false);
    IlluminateNode(const IlluminateNode* node);
};

class SolarNode : public LoopNode
{
public:
    SolarNode(bool raw=false);
    SolarNode(const SolarNode* node);
};

class GetArrayNode : public DNode
{
public:
    GetArrayNode(bool raw=false);
    GetArrayNode(const GetArrayNode* node);
};

class SetArrayNode : public DNode
{
public:
    SetArrayNode(bool raw=false);
    SetArrayNode(const SetArrayNode *node);
};

class ComposeArrayNode : public DNode
{
public:
    ComposeArrayNode(bool raw=false);
    ComposeArrayNode(const ComposeArrayNode* node);

};

class VarNameNode : public DNode
{
public:
    VarNameNode(bool raw=false);
    VarNameNode(const VarNameNode* node);
};

class WriteFileNode : public DNode
{
public:
    WriteFileNode(bool raw=false);
    WriteFileNode(const WriteFileNode* node);
};

class ReadFileNode : public DNode
{
public:
    ReadFileNode(bool raw=false);
    ReadFileNode(const ReadFileNode* node);
};

class ProcessNode : public DNode
{
public:
    ProcessNode(bool raw=false);
    ProcessNode(const ProcessNode* node);
};

class LoadImageNode : public DNode
{
public:
    LoadImageNode(bool raw=false);
    LoadImageNode(const LoadImageNode* node);
};

class SaveImageNode : public DNode
{
public:
    SaveImageNode(bool raw=false);
    SaveImageNode(const SaveImageNode* node);
};
}

#endif // DATA_NODE_H
