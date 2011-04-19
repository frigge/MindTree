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


#ifndef NODES_H
#define NODES_H

#include "QGraphicsItem"
#include "QGraphicsTextItem"
#include "QGraphicsView"
#include "QObject"
#include "QHash"
#include "QPushButton"
#include "QCheckBox"
#include "QGridLayout"

typedef enum
{
    NORMAL,
    VECTOR,
    FLOAT,
    COLOR,
    POINT,
    STRING,
    VARIABLE,
    CONDITION
}socket_type;

typedef enum
{
    IN,
    OUT
} socket_dir;

struct V_NSocket
{
    QString name;
    socket_type type;
    socket_dir dir;
    QGraphicsItem *node;
    QList<V_NSocket*> cntdSockets;
    bool isToken;
    QList<QGraphicsItem *> links;
    QString varname;
};

//typedef V_NSocketData V_NSocket;

typedef QList<V_NSocket> V_NSockets;

class NSocket : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    static int count;
    enum {Type = UserType + 1};
    NSocket(V_NSocket SocketData);
    V_NSocket Socket;
    bool isVariable;
    ~NSocket();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const {return Type;}
    int ID;

    void addLink(QGraphicsItem *nodeLink);
    void clearLinks(bool keep);
    void removeLink(QGraphicsItem *Link);

    static QHash<int, NSocket*>loadIDMapper;
    static QHash<QString, int>SocketNameCnt;

public slots:
    void changeType();
    void changeName();

signals:
    void removeLinkfromSpace(QGraphicsItem *link);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void createContextMenu();
    QMenu *contextMenu;
};

QDataStream & operator<<(QDataStream &stream, NSocket *socket);
QDataStream & operator>>(QDataStream &stream, NSocket **socket);

typedef QList<NSocket*> V_NSocket_ptrs;

enum NType
{
    CONTAINER,          FUNCTION,
    MULTIPLY,           DIVIDE,
    ADD,                SUBTRACT,
    DOTPRODUCT,
    GREATERTHAN,        SMALLERTHAN,
    EQUAL,              AND,OR,
    CONDITIONCONTAINER, NOT,
    FOR,WHILE,          RSLLOOP,

    SURFACEINPUT,       SURFACEOUTPUT,
    DISPLACEMENTINPUT,  DISPLACEMENTOUTPUT,
    VOLUMEINPUT,        VOLUMEOUTPUT,
    LIGHTINPUT,         LIGHTOUTPUT,

    INSOCKETS,          OUTSOCKETS,
    COLORNODE,          FLOATNODE,
    STRINGNODE,         VECTORNODE
};

class NodeName : public QGraphicsTextItem
{
public:
    NodeName(QString name, QGraphicsItem *parent);

protected:
    void focusOutEvent(QFocusEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
};

class Node : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    enum {Type = UserType + 2};
    int ID;
    static int count;

    NodeName *node_name;
    V_NSocket_ptrs *N_outSockets;
    V_NSocket_ptrs *N_inSockets;
    NType NodeType;
    void setDynamicSocketsNode(socket_dir dir, socket_type t = VARIABLE);
    void clearSocketLinks(bool keep);

    NSocket *varsocket;
    NSocket *lastsocket;
    int varcnt;

    Node();
    Node(QString name);
    ~Node();
    virtual void initNode();
    virtual Node *copyNode(QHash<NSocket *, NSocket *> *socketmapping = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * = 0);
    int type() const {return Type;}
    void setNodeName(QString name);
    void setNodeType(NType t);
    void setSockets(V_NSockets inSockets, socket_dir dir);
    void addSocket(NSocket *socket, socket_dir dir);
    void removeSocket(NSocket *socket);
    virtual void dec_var_socket(NSocket *socket);
    virtual void inc_var_socket();

    void setsurfaceOutput();
    void setdisplacementOutput();
    void setvolumeOutput();
    void setlightOutput();
    void setsurfaceInput();
    void setdisplacementInput();
    void setlightInput();
    void setvolumeInput();

    static Node *newNode(QString name, NType t, int ID, QPointF pos, int insize, int outsize);
    static QHash<int, Node*>loadIDMapper;

    static bool isInput(Node *node);
    static bool isMathNode(Node *node);
    static bool isValueNode(Node *node);

protected:
    void drawName();

    virtual int NodeWidth() const;
    virtual int NodeHeight(int numSockets) const;
    virtual void setSocketVarName(NSocket *socket);
};

QDataStream &operator<<(QDataStream &stream, Node *node);
QDataStream &operator>>(QDataStream &stream, Node **node);

class FunctionNode : public Node
{
public:
    FunctionNode();
    QString function_name;
    void setFunctionName(QString name) {function_name = name;}
};

class ContainerNode : public Node
{
    Q_OBJECT
public:
    ContainerNode(NType t = CONTAINER);
    ContainerNode(QString name, NType t = CONTAINER);
    virtual Node *copyNode(QHash<NSocket *, NSocket *> *socketmapping);
    virtual void initNode();

    QGraphicsScene *ContainerData;
    void setContainerData(QGraphicsScene *space){ContainerData = space;}
    void C_addItems(QList<Node*> nodes);

    Node *inSocketNode, *outSocketNode;
    void setInputs(Node *inputNode);
    void setOutputs(Node *outputNode);

    QHash<NSocket*, NSocket*> socket_map;

public slots:
    void newSocket(NSocket *socket);
    void killSocket(NSocket *socket);

private:
    void createContextMenu();
    QMenu *contextMenu;


private slots:
    void addtolib();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void setSocketVarName(NSocket *socket);
};

class SocketNode : public Node
{
    Q_OBJECT
public:
    SocketNode(socket_dir dir, ContainerNode *contnode);

    void setInSocketNode(ContainerNode *contnode);
    void setOutSocketNode(ContainerNode *contnode);

    virtual void inc_var_socket();
    virtual void dec_var_socket(NSocket *socket);

    void connectToContainer(ContainerNode*);

public slots:
    void add_socket(NSocket *socket);
    void remove_socket(NSocket *socket);

signals:
    void socket_added(NSocket *socket);
    void socket_removed(NSocket *socket);

};

class ConditionNode : public Node
{
public:
    ConditionNode(NType t);
    ConditionNode(ConditionNode *node);

    virtual void initNode();
};

class MathNode : public Node
{
    Q_OBJECT
public:
    MathNode(NType t);
    MathNode(NType t, MathNode *node);

    virtual void inc_var_socket();
    virtual void dec_var_socket(NSocket *socket);

    void setSocketType(socket_type t, QList<Node*> *checked_nodes = 0);

signals:
    void SocketTypeChanged(socket_type t);
};

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    ColorButton();

public slots:
    void setColor();

signals:
    void clicked(QColor);
};

class ValueNode : public Node
{
    Q_OBJECT
public slots:
    void setShaderInput(bool isInput);

public:
    virtual void initNode();
    bool isShaderInput;

    void setValueEditor(QWidget *editor);

protected:
    QGraphicsProxyWidget *proxy;
    virtual int NodeWidth() const;
    virtual int NodeHeight(int numSockets) const;
    QWidget *widget;
    QWidget *base_widget;
    QCheckBox *shader_parameter;
    QGridLayout *lay;
    QMenu *contextMenu;
    void createContextMenu();
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class ColorValueNode : public ValueNode
{
    Q_OBJECT
public slots:
    void setValue(QColor);

public:
    ColorValueNode();
    QColor colorvalue;

protected:
    void updateColorLabel();
};

class StringValueNode : public ValueNode
{
    Q_OBJECT
public slots:
    void setValue(QString);

public:
    StringValueNode();
    QString stringvalue;
};

class FloatValueNode : public ValueNode
{
    Q_OBJECT
public slots:
    void setValue(double);

public:
    FloatValueNode();
    float floatvalue;
};

typedef struct
{
    float x;
    float y;
    float z;
}   vector;

class VectorValueNode : public ValueNode
{
    Q_OBJECT
public slots:
    void setValue();

public:
    VectorValueNode();
    vector vectorvalue;
};

class RSLLoopNode : public ContainerNode
{
public:
    RSLLoopNode();
    virtual void initNode();
    QString function_name;
    void setFunctionName(QString name) {function_name = name;}
};

class OutputNode : public Node
{
    Q_OBJECT
public:
    OutputNode();
    QString ShaderName;

private:
    void createMenu();
    QMenu *contextMenu;
    QString filename;

public slots:
    void writeCode();
    void changeName();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void setSocketVarName(NSocket *socket);
};

class InputNode : public Node
{
public:
    InputNode();

protected:
    virtual void setSocketVarName(NSocket *socket);
};

#endif // NODES_H
