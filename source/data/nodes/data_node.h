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

#include "source/data/nodes/data_node_socket.h"

#include "QPointF"
#include "QColor"

#include "source/data/callbacks.h"

class DNode;
class LoadNodeIDMapper
{
public:
    static unsigned short getID(DNode *node);
    static void setID(DNode *node, unsigned short ID);
    static DNode* getNode(unsigned short ID);
    static void clear();

private:
    static QHash<unsigned short, DNode*>loadIDMapper;
};

class CopyNodeMapper
{
public:
    static void setNodePair(DNode *original, DNode *copy);
    static DNode * getCopy(DNode *original);

private:
    static QHash<DNode*, DNode*> nodeMap;
};

enum NType
{
    CONTAINER,          FUNCTION,
    MULTIPLY,           DIVIDE,
    ADD,                SUBTRACT,
    DOTPRODUCT,

    GREATERTHAN,        SMALLERTHAN,
    EQUAL,              AND,OR,
    CONDITIONCONTAINER, NOT,

    FOR, WHILE, GATHER, ILLUMINANCE,
    ILLUMINATE, SOLAR,

    SURFACEINPUT,       SURFACEOUTPUT,
    DISPLACEMENTINPUT,  DISPLACEMENTOUTPUT,
    VOLUMEINPUT,        VOLUMEOUTPUT,
    LIGHTINPUT,         LIGHTOUTPUT,
    ILLUMINANCEINPUT,   ILLUMINATEINPUT,
    SOLARINPUT,

    INSOCKETS,          OUTSOCKETS,
    LOOPINSOCKETS,      LOOPOUTSOCKETS,

    COLORNODE,          FLOATNODE,
    INTNODE, BOOLNODE,
    STRINGNODE,         VECTORNODE,
    PREVIEW, GETARRAY,  SETARRAY,
    COMPOSEARRAY,       VARNAME,
    OBJECTNODE,         POLYGONNODE,
    FLOATTOVECTOR
};

class VNode;
class DNSpace;
class ContainerNode;

typedef QList<DNode*> NodeList;
typedef QList<const DNode*> ConstNodeList;

class DNode
{
public:
    DNode(QString name="");
    DNode(const DNode* node);
    static DNode* copy(const DNode *original);
    static QList<DNode*> copy(QList<DNode*>nodes);
    virtual ~DNode();

    template<class C>
    const C* getDerivedConst() const;

    template<class C>
    C* getDerived();

    NodeList getAllInNodes(NodeList nodes=NodeList());
    NodeList getAllInNodesConst(NodeList nodes=NodeList()) const;

    virtual VNode* createNodeVis();
    virtual void deleteNodeVis();
    virtual void setNodeName(QString name);
    QString getNodeName() const;
    void setNodeType(NType t);
    virtual void addSocket(DSocket *socket);
    QStringList getSocketNames();
    void setSocketIDName(DSocket *socket);
    DSocket* getSocketByIDName(QString idname);
    void removeSocket(DSocket *socket);
    virtual void dec_var_socket(DSocket *socket);
    virtual void inc_var_socket();
    void setDynamicSocketsNode(socket_dir dir, socket_type t=VARIABLE);
    void clearSocketLinks();
    bool isContainer() const;

    void setNodeVis(VNode* value);
    VNode* getNodeVis() const;
    QPointF getPos() const;
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

    void regAddSocketCB(Callback *cb);
    void remAddSocketCB(Callback *cb);
    void unblockCB();
    void blockCB();
    void unblockRegCB();
    void blockRegCB();

    static void setsurfaceOutput(DNode *node);
    static void setdisplacementOutput(DNode *node);
    static void setvolumeOutput(DNode *node);
    static void setlightOutput(DNode *node);
    static void setsurfaceInput(DNode *node);
    static void setdisplacementInput(DNode *node);
    static void setlightInput(DNode *node);
    static void setvolumeInput(DNode *node);
    static void setIlluminanceInput(DNode *node);
    static void setIlluminateInput(DNode *node);

    static DNode *newNode(QString name, NType t, int insize, int outsize);

    static bool isInput(const DNode *node);
    static bool isMathNode(const DNode *node);
    static bool isConditionNode(const DNode *node);
    bool isValueNode()const;
    
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;

    bool isGhost();

    DNode *createFuncNode(QString filepath);
    static DNode *dropNode(QString filepath);
    static ContainerNode *buildContainerNode(QList<DNode*>nodes);
    static void unpackContainerNode(DNode *node);

    void addArray(DSocket *first);

private:
    DNSpace *space;
    VNode* nodeVis;
    DSocket *varsocket;
    DSocket *lastsocket;
    bool ghost;
    bool blockCBregister;
    int varcnt;
    unsigned short ID;
    static unsigned short count;
    QString nodeName;
    mutable DSocketList outSockets;
    mutable DSocketList inSockets;
    NType NodeType;
    QList<ArrayContainer*> arrays;
    CallbackList addSocketCallbacks;
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
QDataStream &operator>>(QDataStream &stream, DNode **node);

class FunctionNode : public DNode
{
public:
    FunctionNode(QString name="");
    FunctionNode(const FunctionNode* node);
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;
    QString getFunctionName() const;
    void setFunctionName(QString value);

private:
    QString function_name;
};

class VContainerNode;
class ContainerSpace;
class SocketNode;

class ContainerNode : public DNode
{
public:
    ContainerNode(QString name="", bool raw=false);
    ContainerNode(const ContainerNode* node);
    ~ContainerNode();
    virtual VNode* createNodeVis();

    DSocket *getSocketInContainer(const DSocket*) const;
    DSocket *getSocketOnContainer(const DSocket*) const;
    QList<DSocket*> getMappedSocketsOnContainer() const;
    void mapOnToIn(DSocket *on, DSocket *in);
    int getSocketMapSize() const;

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

    void setNodeName(QString name);
    virtual bool operator==(const DNode &node)const;
    virtual bool operator!=(const DNode &node)const;

    void newSocket(DSocket *socket);
    void killSocket(DSocket *socket);
    void addtolib();

private:
    SocketNode *inSocketNode, *outSocketNode;
    ContainerSpace *containerData;
    QHash<DSocket*, DSocket*> socket_map;
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

class MathNode : public DNode
{
public:
    MathNode(NType t, bool raw=false);
    MathNode(const MathNode *node);

    virtual void dec_var_socket(DSocket *socket);
};

class ValueNode : public DNode
{
public:
    ValueNode(QString name);
    ValueNode(const ValueNode* node);
    bool isShaderInput() const;
    void setShaderInput(bool);

    void setNodeName(QString name);

protected:
    //virtual VNode* createNodeVis();
    //virtual void getValue() = 0;

private:
    bool shaderInput;
};

class ColorValueNode : public ValueNode
{
public:
    ColorValueNode(QString name="Color", bool raw=false);
    ColorValueNode(const ColorValueNode* node);
    //virtual bool operator==(const DNode &node)const;
    //virtual bool operator!=(const DNode &node)const;

    //void setValue(QColor);
    //QColor getValue() const;

//protected:
    //void updateColorLabel();
    //virtual VNode* createNodeVis();

//private:
    //QColor colorvalue;
};

class StringValueNode : public ValueNode
{
public:
    StringValueNode(QString name="String", bool raw=false);
    StringValueNode(const StringValueNode* node);
};

class FloatValueNode : public ValueNode
{
public:
    FloatValueNode(QString name="Float", bool raw=false);
    FloatValueNode(const FloatValueNode* node);
};

class IntValueNode : public ValueNode
{
public:
    IntValueNode(QString name="Integer", bool raw=false);
    IntValueNode(const IntValueNode* node);
};

class BoolValueNode : public ValueNode
{
public:
    BoolValueNode(QString name="Boolean", bool raw=false);
    BoolValueNode(const BoolValueNode* node);
};

class VectorValueNode : public ValueNode
{
public:
    VectorValueNode(QString name="Vector", bool raw=false);
    VectorValueNode(const VectorValueNode* node);
    //Vector getValue() const;
    //void setValue(Vector newvalue);
    //virtual bool operator==(const DNode &node)const;
    //virtual bool operator!=(const DNode &node)const;
    //VNode* createNodeVis();

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
    LoopNode(QString name="", bool raw=false);
    LoopNode(const LoopNode* node);
    static bool isLoopNode(DNode *);
    void setLoopedSockets(LoopSocketNode *node);

private:
    SocketNode *loopSockets, *initSockets;
};

class ForNode : public LoopNode
{
public:
    ForNode(bool raw=false);
    ForNode(const ForNode* node);
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

class SourceDock;
class OutputNode : public DNode
{
public:
    OutputNode();
    OutputNode(const OutputNode* node);
    virtual ~OutputNode();
    QString getShaderName() const;

    void writeCode();
    SourceDock* getSourceEdit();
    void compile();
    void changeName(QString);
    QString getFileName() const;
    void setFileName(QString);

protected:
    virtual VNode* createNodeVis();

private:
    QString filename;
    QString ShaderName;
    SourceDock *sedit;
};

class InputNode : public DNode
{
public:
    InputNode();
    InputNode(const InputNode* node);
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

#endif // DATA_NODE_H
