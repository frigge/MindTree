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


#include "QPainter"
#include "QStyleOption"
#include "QHashIterator"
#include "QGraphicsSceneMouseEvent"i
#include "QGraphicsEffect"
#include "QVariant"
#include "QLabel"
#include  "QLayout"
#include "QColorDialog"
#include "QDoubleSpinBox"
#include "QInputDialog"
#include "QCoreApplication"
#include "QFileDialog"
#include "QTextStream"
#include "QTextCursor"

#include "node.h"
#include "nodelink.h"
#include "shader_space.h"
#include "shaderwriter.h"

QHash<int, NSocket*> NSocket::loadIDMapper;
QHash<QString, int> NSocket::SocketNameCnt;
int NSocket::count = 0;

NSocket::NSocket(V_NSocket SocketData)
{
    ID = count;
    count++;
    Socket = SocketData;
    setAcceptDrops(true);
    setAcceptHoverEvents(true);
    setZValue(zValue()+0.1);
    setFlag(ItemIsSelectable, false);
    isVariable = false;
//    setGraphicsEffect(new QGraphicsDropShadowEffect);
    createContextMenu();

};

NSocket::~NSocket()
{
    disconnect();
}

void NSocket::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *changeNameAction = contextMenu->addAction("Rename Socket");
    QAction *changeTypeAction = contextMenu->addAction("change Type");

    connect(changeNameAction, SIGNAL(triggered()), this, SLOT(changeName()));
    connect(changeTypeAction, SIGNAL(triggered()), this, SLOT(changeType()));
}

void NSocket::changeName()
{
    bool ok;
    QString newname;
    newname = QInputDialog::getText(this->scene()->views().first(), "Change Socket Name", "New Name", QLineEdit::Normal, "", &ok);
    if(ok)
        Socket.name = newname;
}

void NSocket::changeType()
{
    socket_type newtype;
    QStringList typelist;
    bool ok;
    typelist<<"Normal" << "Vector"<<"Float"<<"Color"<< "Point"<<"String"<<"Variable";
    QString newtypestr(QInputDialog::getItem(this->scene()->views().first(), "Change Socket Type", "New Type", typelist, 0, false, &ok));

    if (newtypestr == "Normal")
        newtype = NORMAL;
    else if (newtypestr == "Vector")
        newtype = VECTOR;
    else if (newtypestr == "Float")
        newtype  = FLOAT;
    else if (newtypestr == "Color")
        newtype = COLOR;
    else if (newtypestr == "Point")
        newtype = POINT;
    else if (newtypestr == "String")
        newtype = STRING;
    else if (newtypestr == "Variable")
        newtype = VARIABLE;

    if(ok)
        Socket.type = newtype;
}

void NSocket::addLink(QGraphicsItem *nodeLink)
{
    if(Socket.links.contains(nodeLink))
        return;
    Socket.links.append(nodeLink);
    NodeLink *nl = (NodeLink *)nodeLink;
    if(nl->inSocket == this)
    {
        V_NSocket *outdata = &nl->outSocket->Socket;
        if(Socket.cntdSockets.contains(outdata))
            return;
        Socket.cntdSockets.append(outdata);
    }
    else
    {
        V_NSocket *indata = &nl->inSocket->Socket;
        if(Socket.cntdSockets.contains(indata))
            return;
        Socket.cntdSockets.append(indata);
    }

    Node *node = (Node*) Socket.node;
    if (Socket.type == VARIABLE)
    {
        Socket.type = Socket.cntdSockets.first()->type;
        if(Socket.dir == IN)Socket.name = Socket.cntdSockets.first()->name;
//        if(MathNode::isMathNode(node))
//        {
//            MathNode *mNode = (MathNode*)node;
//            mNode->setSocketType(Socket.type);
//        }
    }
    if (isVariable)
        node->inc_var_socket();
}

QDataStream & operator<<(QDataStream &stream, NSocket *socket)
{
    stream<<(qint16)socket->ID<<socket->isVariable<<socket->Socket.dir;
    stream<<socket->Socket.isToken<<socket->Socket.name<<socket->Socket.type;
    return stream;
}

QDataStream & operator>>(QDataStream &stream, NSocket **socket)
{
    int dir, type;
    qint16 ID;
    V_NSocket sdata;
    bool isVar;
    stream>>ID;
    stream>>isVar;
    stream>>dir;
    stream>>sdata.isToken;
    stream>>sdata.name;
    stream>>type;
    sdata.dir = (socket_dir)dir;
    sdata.type = (socket_type)type;
    NSocket *newsocket = new NSocket(sdata);
    newsocket->isVariable = isVar;
    *socket = newsocket;
    NSocket::loadIDMapper.insert(ID, *socket);
    return stream;
}

void NSocket::removeLink(QGraphicsItem *Link)
{
    NodeLink *nl = (NodeLink*)Link;
    Node *pnode = (Node*)Socket.node;
    if(Socket.dir == IN)
    {
        Socket.cntdSockets.clear();
        Socket.links.clear();
    }
    else
    {
        Socket.links.removeAll(Link);
        Socket.cntdSockets.removeAll(&nl->inSocket->Socket);
    }

    if(isVariable)
        pnode->dec_var_socket(this);
    if((pnode->NodeType == ADD
       || pnode->NodeType == SUBTRACT
       || pnode->NodeType == MULTIPLY
       || pnode->NodeType == DIVIDE)
        && Socket.dir == OUT
        && pnode->varcnt == 0)
    {
        pnode->varsocket->Socket.type = VARIABLE;
        Socket.type = VARIABLE;
    }

}

void NSocket::clearLinks(bool keep)
{
    Socket.cntdSockets.clear();
    foreach(QGraphicsItem *qglink, Socket.links)
    {
        if (qglink)
        {
            NodeLink *nlink =(NodeLink*)qglink;
            if(Socket.dir == IN)
                nlink->inSocket = 0;
            else
                nlink->outSocket = 0;
            if(!keep)
                emit removeLinkfromSpace(nlink);
        }
    }
}

QRectF NSocket::boundingRect() const
{
    return QRectF(-15, 15, -15, 15);
};

void NSocket::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    QColor color;
    switch(Socket.type)
    {
    case STRING:
        if (isUnderMouse())
            color = QColor(255, 100, 100);
        else
            color = QColor(160, 60, 60);
        break;
    case POINT:
        if (isUnderMouse())
            color = QColor(0, 255, 255);
        else
            color = QColor(90, 140, 210, 100);
        break;
    case NORMAL:
        if (isUnderMouse())
            color = QColor(0, 255, 255);
        else
            color = QColor(90, 140, 210, 100);
        break;
    case FLOAT:
        if (isUnderMouse())
            color = QColor(0, 255, 0);
        else
            color = QColor(90, 200, 90, 100);
        break;
    case VECTOR:
        if (isUnderMouse())
            color = QColor(0, 255, 255);
        else
            color = QColor(90, 140, 210, 100);
        break;
    case COLOR:
        if (isUnderMouse())
            color = QColor(255, 255, 0);
        else
            color = QColor(210, 210, 80, 100);
        break;
    case CONDITION:
        if (isUnderMouse())
            color = QColor(255, 0, 0);
        else
            color = QColor(170, 50, 50, 100);
        break;
    case VARIABLE:
        if(isUnderMouse())
            color = QColor(255, 0, 255);
        else
            color = QColor(120, 70, 120, 100);
    };
    painter->setBrush(QBrush(color, Qt::SolidPattern));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(-15, 15, -15, 15, 2.5, 2.5);
};

void NSocket::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    update();
};

void NSocket::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    update();
};

void NSocket::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Node *node = (Node*)Socket.node;
    Shader_Space *space = (Shader_Space*)node->scene();

    if(event->button() != Qt::LeftButton)
        return;

    if (space->newlink == 0)
    {
        space->enterlinkNodeMode(this);
    }
    else
    {
        space->leavelinkNodeMode(event->scenePos());
    }
}

void NSocket::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}

NodeName::NodeName(QString name, QGraphicsItem *parent)
{
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsFocusable, true);

    setPlainText(name);
    setParentItem(parent);
}

void NodeName::focusOutEvent(QFocusEvent *event)
{
    QGraphicsTextItem::focusOutEvent(event);
    Node *pNode = (Node*)parentItem();
    if((pNode->isContainer()))
    {
        ContainerNode *cNode = (ContainerNode*)pNode;
        if(cNode->ContainerData)
        {
            Shader_Space *contspace = (Shader_Space*)cNode->ContainerData;
            contspace->setName(toPlainText());
        }
    }
    if(scene())
    {
        Shader_Space *space = (Shader_Space*)scene();
        space->leaveEditNameMode();
    }
    setTextInteractionFlags(Qt::NoTextInteraction);
    textCursor().clearSelection();
}

void NodeName::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(scene())
    {
        Shader_Space *space = (Shader_Space*)scene();
        space->enterEditNameMode();
    }
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

int Node::count = 1;
QHash<int, Node*>Node::loadIDMapper;

Node::Node()
{
    initNode();
}

Node::Node(QString name)
{
    initNode();
    setNodeName(name);
};

Node::~Node()
{
    foreach(NSocket *socket, *N_inSockets)
        removeSocket(socket);
    foreach(NSocket *socket, *N_outSockets)
        removeSocket(socket);

    delete N_inSockets;
    delete N_outSockets;
}

Node* Node::copyNode(QHash<NSocket *, NSocket *> *socketmapping)
{
    Node *newNode = new Node(node_name->toPlainText());
    foreach(NSocket *socket, *N_inSockets)
    {
        NSocket *newsocket = new NSocket(socket->Socket);
        socketmapping->insert(socket, newsocket);
        socket->Socket.cntdSockets.clear();
        newNode->addSocket(newsocket, socket->Socket.dir);
    }

    foreach(NSocket *socket, *N_outSockets)
    {
        NSocket *newsocket = new NSocket(socket->Socket);
        socketmapping->insert(socket, newsocket);
        newsocket->Socket.cntdSockets.clear();
        newNode->addSocket(newsocket, socket->Socket.dir);
    }

    newNode->setNodeType(NodeType);
    return newNode;
}

bool Node::isContainer()
{
    if(NodeType == CONTAINER
            ||NodeType == CONDITIONCONTAINER
            ||NodeType == FOR
            ||NodeType == WHILE
            ||NodeType == GATHER
            ||NodeType == ILLUMINANCE
            ||NodeType == ILLUMINATE
            ||NodeType == SOLAR)
        return true;
    else return false;
}

QDataStream &operator <<(QDataStream &stream, Node  *node)
{
    stream<<node->node_name->toPlainText()<<(qint16)node->ID<<node->NodeType<<node->pos();
    stream<<(qint16)node->N_inSockets->size()<<(qint16)node->N_outSockets->size();
    foreach(NSocket *socket, *node->N_inSockets)
        stream<<socket;
    foreach(NSocket *socket, *node->N_outSockets)
        stream<<socket;

    if(node->NodeType == FUNCTION)
    {
        FunctionNode *fnode = (FunctionNode*) node;
        stream<<fnode->function_name;
    }

    if(node->isContainer())
    {
        ContainerNode *cnode =(ContainerNode*) node;
        stream<<cnode->inSocketNode->ID<<cnode->outSocketNode->ID;
        stream<<cnode->socket_map.size();
        foreach(NSocket *socket, cnode->socket_map.keys())
            stream<<socket->ID<<cnode->socket_map[socket]->ID;
        Shader_Space *cdata = (Shader_Space*) cnode->ContainerData;
        stream<<cdata;
    }

    if(node->NodeType == INSOCKETS
            ||node->NodeType == OUTSOCKETS)
    {
        SocketNode *snode = (SocketNode*)snode;
        stream<<(int)snode->loopSocket();
        if(snode->loopSocket())
        {
            LoopSocketNode *lsnode = (LoopSocketNode*)node;
            stream<<lsnode->loopSocketMap.size();
            foreach(NSocket *socket, lsnode->loopSocketMap)
            {
                stream<<socket->ID<<lsnode->loopSocketMap.value(socket)->ID;
            }
        }

    }

    if(node->NodeType == COLORNODE)
    {
        ColorValueNode *colornode = (ColorValueNode*)node;
        stream<<colornode->colorvalue;
    }

    if(node->NodeType == FLOATNODE)
    {
        FloatValueNode *floatnode = (FloatValueNode*)node;
        stream<<floatnode->floatvalue;
    }

    if(node->NodeType == STRINGNODE)
    {
        StringValueNode *stringnode = (StringValueNode*)node;
        stream<<stringnode->stringvalue;
    }

    if(node->NodeType == VECTORNODE)
    {
        VectorValueNode *vectornode = (VectorValueNode*)node;
        stream<<vectornode->vectorvalue.x<<vectornode->vectorvalue.y<<vectornode->vectorvalue.z;
    }

    return stream;
}

Node *Node::newNode(QString name, NType t, int ID, QPointF pos, int insize, int outsize, bool isLoopSocket)
{
    Node *node;
    if(t==CONTAINER)
        node = new ContainerNode(true);
    else if (t == FOR)
        node = new ForNode(true);
    else if (t == WHILE)
        node = new WhileNode(true);
    else if (t == GATHER)
        node = new GatherNode(true);
    else if (t == SOLAR)
        node = new SolarNode(true);
    else if (t == ILLUMINATE)
        node = new IlluminateNode(true);
    else if (t == ILLUMINANCE)
        node = new IlluminanceNode(true);
    else if (t == CONDITIONCONTAINER)
        node = new ConditionContainerNode;
    else if(t == FUNCTION)
        node = new FunctionNode();
    else if(t == MULTIPLY
            ||t == DIVIDE
            ||t == ADD
            ||t == SUBTRACT
            ||t == DOTPRODUCT)
        node = new MathNode(t);
    else if(t == GREATERTHAN
            ||t == SMALLERTHAN
            ||t == EQUAL
            ||t == AND
            ||t == OR
            ||t == NOT)
        node = new ConditionNode(t);
    else if(t == COLORNODE)
        node = new ColorValueNode();
    else if(t == FLOATNODE)
        node = new FloatValueNode();
    else if(t == STRINGNODE)
        node = new StringValueNode();
    else if(t == VECTORNODE)
        node = new VectorValueNode();
    else if(t == INSOCKETS
            ||t == OUTSOCKETS)
    {
        if(isLoopSocket)
            node = new SocketNode(insize == 0 ? OUT : IN, 0);
        else
            node = new LoopSocketNode(insize == 0 ? OUT : IN, 0);
    }
    else if(t == SURFACEOUTPUT
            ||t == DISPLACEMENTOUTPUT
            ||t == VOLUMEOUTPUT
            ||t == LIGHTOUTPUT)
        node = new OutputNode();
    else if(t == SURFACEINPUT
            ||t == DISPLACEMENTINPUT
            ||t == VOLUMEINPUT
            ||t == LIGHTINPUT)
        node = new InputNode();
    else
        node = new Node();

    node->setNodeName(name);
    node->setNodeType(t);
    node->setPos(pos);
    Node::loadIDMapper.insert(ID, node);
    return node;
}

QDataStream &operator >>(QDataStream &stream, Node  **node)
{
    QString name;
    qint16 ID, insocketsize, outsocketsize;
    int nodetype;
    QPointF nodepos;
    stream>>name>>ID>>nodetype>>nodepos;
    stream>>insocketsize>>outsocketsize;
    bool isloopsocket = false;
    if(nodetype == INSOCKETS
            || nodetype == OUTSOCKETS)
        stream>>isloopsocket;

    Node *newnode;
    newnode = Node::newNode(name, (NType)nodetype, ID, nodepos, insocketsize, outsocketsize, isloopsocket);
    *node = newnode;

    NSocket *socket;
    for(int i=0; i<insocketsize; i++)
    {
        stream>>&socket;
        newnode->addSocket(socket, IN);
        if(socket->isVariable) newnode->varsocket = socket;
    }
    for(int j=0; j<outsocketsize; j++)
    {
        stream>>&socket;
        newnode->addSocket(socket, OUT);
        if(socket->isVariable) newnode->varsocket = socket;
    }

    if(newnode->NodeType == FUNCTION)
    {
        FunctionNode *fnode = (FunctionNode*) newnode;
        stream>>fnode->function_name;
    }

    int inSocketID;
    int outSocketID;
    int smapsize, keyID, valueID;
    if(newnode->isContainer())
    {
        ContainerNode *contnode = (ContainerNode*)newnode;
        stream>>inSocketID>>outSocketID;
        stream>>smapsize;
        QPair<int, int>cont_socket_map_ID_mapper[smapsize];
        for(int i = 0; i < smapsize; i++)
        {
            stream>>keyID>>valueID;
            cont_socket_map_ID_mapper[i].first = keyID;
            cont_socket_map_ID_mapper[i].second = valueID;
        }
        Shader_Space *space;
        stream>>&space;
        contnode->setContainerData(space);
        contnode->inSocketNode = Node::loadIDMapper[inSocketID];
        contnode->outSocketNode = Node::loadIDMapper[outSocketID];
        SocketNode *innode = (SocketNode*)contnode->inSocketNode;
        SocketNode *outnode = (SocketNode*)contnode->outSocketNode;
        innode->connectToContainer(contnode);
        outnode->connectToContainer(contnode);
        for(int j = 0; j < smapsize; j++)
        {
            keyID = cont_socket_map_ID_mapper[j].first;
            valueID = cont_socket_map_ID_mapper[j].second;
            contnode->socket_map.insert(NSocket::loadIDMapper[keyID], NSocket::loadIDMapper[valueID]);
        }
        if(LoopNode::isLoopNode(newnode))
        {
            LoopSocketNode *loutnode = (LoopSocketNode*) outnode;
            LoopSocketNode *linnode = (LoopSocketNode*) innode;
            loutnode->setPartner(linnode);
        }
    }

    if(newnode->NodeType == COLORNODE)
    {
        ColorValueNode *colornode = (ColorValueNode*)*node;
        stream>>colornode->colorvalue;
    }

    if(newnode->NodeType == FLOATNODE)
    {
        FloatValueNode *floatnode = (FloatValueNode*)*node;
        stream>>floatnode->floatvalue;
    }

    if(newnode->NodeType == STRINGNODE)
    {
        StringValueNode *stringnode = (StringValueNode*)*node;
        stream>>stringnode->stringvalue;
    }

    if(newnode->NodeType == VECTORNODE)
    {
        VectorValueNode *vectornode = (VectorValueNode*)newnode;
        stream>>vectornode->vectorvalue.x>>vectornode->vectorvalue.y>>vectornode->vectorvalue.z;
    }

    if(newnode->NodeType == INSOCKETS
            ||newnode->NodeType == OUTSOCKETS)
    {
        SocketNode *snode = (SocketNode*) newnode;
        snode->setLoopSocket(isloopsocket);
        if(isloopsocket)
        {
            LoopSocketNode *lsnode = (LoopSocketNode*)snode;
            int partnerSockets, socketID, partnerID;
            stream>>partnerSockets;
            for(int i = 0; i < partnerSockets; i++)
            {
                stream>>socketID>>partnerID;
                NSocket *socket = NSocket::loadIDMapper[socketID];
                NSocket *partner = NSocket::loadIDMapper[partnerID];
                lsnode->loopSocketMap.insert(socket, partner);
            }
        }
    }

    return stream;
}

void Node::initNode()
{
    varcnt = 0;
    ID = count;
    count++;
    node_name = new NodeName("", this);
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsFocusable, true);
    setFlag(ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);

    QGraphicsDropShadowEffect *dropshad = new QGraphicsDropShadowEffect;
    dropshad->setBlurRadius(10);
    dropshad->setOffset(5);
    dropshad->setColor(QColor(10, 10, 10, 200));
    setGraphicsEffect(dropshad);

    N_inSockets = new V_NSocket_ptrs;
    N_outSockets = new V_NSocket_ptrs;

    node_name->setDefaultTextColor(QColor(255, 255, 255));
}

void Node::setNodeType(NType t)
{
    NodeType = t;
}

void Node::setNodeName(QString name)
{
    node_name->setPlainText(name);
}

void Node::setSockets(V_NSockets Sockets, socket_dir dir)
{
    foreach(V_NSocket sdata, Sockets)
    {
        NSocket *socket = new NSocket(sdata);
        addSocket(socket, dir);
    }
}

void Node::addSocket(NSocket *socket, socket_dir dir)
{
    socket->setParentItem(this);
    socket->Socket.node = this;
    socket->Socket.dir = dir;
    socket->Socket.cntdSockets.clear();
    socket->Socket.links.clear();
    if(dir==IN) N_inSockets->append(socket);
    else N_outSockets->append(socket);
    drawName();
    Shader_Space *space = (Shader_Space*)scene();
//    connect(socket, SIGNAL(removeLinkfromSpace(QGraphicsItem*)), space, SLOT(removeLink(NodeLink*)));
    setSocketVarName(socket);
}

void Node::removeSocket(NSocket *socket)
{
    socket_dir dir = socket->Socket.dir;
    if(dir == IN)
        N_inSockets->removeAll(socket);
    else
        N_outSockets->removeAll(socket);
    socket->clearLinks(false);
    delete socket;
    drawName();
}

void Node::dec_var_socket(NSocket *socket)
{
    removeSocket(socket);
    varcnt -= 1;
}

void Node::drawName()
{
    int node_width = NodeWidth();
    int node_height = NodeHeight(N_inSockets->size() + N_outSockets->size());
    node_name->setPos(0-(node_width/2)+4, -(node_height/2)-20);
}

void Node::inc_var_socket()
{
    QList<V_NSocket *> *csocks = &varsocket->Socket.cntdSockets;
//    if(csocks->size() > 0) varsocket->Socket.type = csocks->first()->type;
//    if(csocks->size() > 0) varsocket->Socket.name = csocks->first()->name;
    V_NSocket socketdata;
    socketdata.node = this;
    socketdata.name = "Add Socket";
    socketdata.type = VARIABLE;
    socketdata.dir = varsocket->Socket.dir;
    lastsocket = varsocket;
    varsocket = new NSocket(socketdata);
    varsocket->isVariable = true;
    addSocket(varsocket, socketdata.dir);
    varcnt +=1;
}

void Node::setSocketVarName(NSocket *socket)
{
    if(socket->Socket.dir == IN)
        return;
    if(NodeType == SURFACEINPUT
       ||NodeType == DISPLACEMENTINPUT
       ||NodeType == VOLUMEINPUT
       ||NodeType == LIGHTINPUT)
    {
        socket->Socket.varname = socket->Socket.name;
        socket->setToolTip(socket->Socket.varname);
        return;
    }

    if((NodeType == CONTAINER
       ||NodeType == CONDITIONCONTAINER)
       && N_inSockets->isEmpty())
    {
        socket->Socket.varname = socket->Socket.name;
        socket->setToolTip(socket->Socket.varname);
        return;
    }

    QString varname_raw = socket->Socket.name.toLower().replace(" ", "_");
    if(NSocket::SocketNameCnt.contains(varname_raw))
    {
        NSocket::SocketNameCnt[varname_raw]++;
        socket->Socket.varname = varname_raw + QString::number(NSocket::SocketNameCnt[varname_raw]);
    }
    else
    {
        NSocket::SocketNameCnt.insert(varname_raw, 1);
        socket->Socket.varname = varname_raw + QString::number(1);
    }
    socket->setToolTip(socket->Socket.varname);
}

void Node::clearSocketLinks(bool keep)
{
    foreach(NSocket *socket, *N_inSockets)
       socket->clearLinks(keep);
    foreach(NSocket *socket, *N_outSockets)
        socket->clearLinks(keep);
}

int Node::NodeWidth() const
{
    int node_width = 150;
    return node_width;
};

int Node::NodeHeight(int numSockets) const
{
    int node_height = 30 + (17*numSockets);
    return node_height;
};

QRectF Node::boundingRect() const
{
    int node_height = NodeHeight(N_inSockets->size() + N_outSockets->size());
    node_height +=2;
    int node_width = NodeWidth();
    node_width += 2;
    return QRectF(0-(node_width/2), 0-(node_height/2), node_width, node_height);
};

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    int node_width = NodeWidth();
    int node_height = NodeHeight(N_inSockets->size() + N_outSockets->size());
    int header_height = 20;
    int space = 2;
    int socket_size = 15;
    int nodePen = 4;
    QColor nodeBGColor;
    QColor nodeHColor;
    QColor textColor;
    QPen node_outline;

    if (isSelected())
    {
        nodeBGColor = QColor(100, 100, 100);
        nodeHColor = QColor(120, 120, 120);
        textColor = QColor(255, 255, 255);
        node_outline.setColor(QColor(255, 145, 0, 100));
        node_outline.setWidth(nodePen);
    }
    else
    {
        nodeBGColor = QColor(100, 100, 100);
        nodeHColor = QColor(100, 100, 100);
        textColor = QColor(255, 255, 255, 255);
        node_outline.setColor(QColor(80, 80, 80, 100));
        node_outline.setWidth(nodePen);
    };
    painter->setPen(node_outline);
    painter->setBrush(QBrush(nodeBGColor, Qt::SolidPattern));
    painter->drawRect(-(node_width/2), 0-(node_height/2), node_width, node_height);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(nodeHColor, Qt::SolidPattern));
    QRectF hrect((0-(node_width/2))+6, 0-(node_height/2)+2, node_width-12, header_height);
    painter->drawRect(hrect);

    int inSocket_pos;
    inSocket_pos = 0 - (node_height/2) + header_height + 2*space;

    foreach(NSocket *in, *N_inSockets)
    {
        NSocket *insocket = in;
        insocket->setPos(2*socket_size-(node_width/2)+2*space, inSocket_pos-socket_size);
        painter->setPen(textColor);
        QRectF textrect(-node_width/2 + (socket_size+4*space), inSocket_pos, node_width-(socket_size+4*space), socket_size);
        painter->drawText(textrect, in->Socket.name, QTextOption(Qt::AlignLeft));
        inSocket_pos += socket_size + space;
    }

    int outSocket_pos;
    outSocket_pos = inSocket_pos + 2;

    foreach(NSocket *out, *N_outSockets)
    {
        out->setPos((node_width/2)+10, outSocket_pos-socket_size);
        painter->setPen(textColor);
        QRectF outtextrect(-node_width/2, outSocket_pos, node_width-(socket_size+4*space), socket_size);
        painter->drawText(outtextrect, out->Socket.name, QTextOption(Qt::AlignRight));
        outSocket_pos += socket_size + space;
    }
};

void Node::setsurfaceInput()
{
    V_NSockets in, out;
    V_NSocket P, N, Cs, Os, u, v, du, dv, s, t, I;
    P.type = POINT;
    P.name = "P";
    N.type = NORMAL;
    N.name = "N";
    Cs.type = COLOR;
    Cs.name = "Cs";
    Os.type = COLOR;
    Os.name = "Os";
    u.type = FLOAT;
    u.name = "u";
    v.type = FLOAT;
    v.name = "v";
    du.type = FLOAT;
    du.name = "du";
    dv.type = FLOAT;
    dv.name = "dv";
    s.type = FLOAT;
    s.name = "s";
    t.type = FLOAT;
    t.name = "t";
    I.type = VECTOR;
    I.name = "I";
    out.append(P);
    out.append(N);
    out.append(Cs);
    out.append(Os);
    out.append(u);
    out.append(v);
    out.append(du);
    out.append(dv);
    out.append(s);
    out.append(t);
    out.append(I);
    setNodeName("Surface Input");
    setNodeType(SURFACEINPUT);
    setSockets(in, IN);
    setSockets(out, OUT);
}

void Node::setdisplacementInput()
{
    V_NSockets in, out;
    V_NSocket P, N, u, v, du, dv, s, t;
    P.type = POINT;
    P.name ="P";
    N.type = NORMAL;
    N.name = "N";
    u.type = FLOAT;
    u.name = "u";
    v.type = FLOAT;
    v.name = "v";
    du.type = FLOAT;
    du.name = "du";
    dv.type = FLOAT;
    dv.name = "dv";
    s.type = FLOAT;
    s.name = "s";
    t.type = FLOAT;
    t.name = "t";
    out.append(P);
    out.append(N);
    out.append(u);
    out.append(v);
    out.append(du);
    out.append(dv);
    out.append(s);
    out.append(t);
    setNodeName("Displacement Input");
    setNodeType(DISPLACEMENTINPUT);
    setSockets(in, IN);
    setSockets(out, OUT);
}

void Node::setvolumeInput()
{
    V_NSockets in, out;
    V_NSocket P, I, Ci, Oi, L, Cl;
    P.type = POINT;
    P.name = "P";
    I.type = VECTOR;
    I.name = "I";
    Ci.type = COLOR;
    Ci.name = "Ci";
    Oi.type = COLOR;
    Oi.name = "Oi";
    L.type = VECTOR;
    L.name = "L";
    Cl.type = COLOR;
    Cl.name = "Cl";
    out.append(I);
    out.append(P);
    out.append(Ci);
    out.append(Oi);
    out.append(L);
    out.append(Cl);
    setNodeName("Volume Input");
    setNodeType(VOLUMEINPUT);
    setSockets(in,  IN);
    setSockets(out,  OUT);
}

void Node::setlightInput()
{
    V_NSockets in, out;
    V_NSocket P, Ps, L;
    P.type = POINT;
    P.name = "P";
    Ps.type = POINT;
    Ps.name = "Ps";
    L.type = VECTOR;
    L.name = "L";
    out.append(P);
    out.append(Ps);
    out.append(L);
    setNodeName("Light Input");
    setNodeType(LIGHTINPUT);
    setSockets(in,  IN);
    setSockets(out,  OUT);
}

void Node::setsurfaceOutput()
{
    V_NSockets in, out;
    V_NSocket Ci, Oi;
    Ci.type = COLOR;
    Ci.name = "Ci";
    Oi.type = COLOR;
    Oi.name = "Oi";
    in.append(Ci);
    in.append(Oi);
    setNodeName("Surface Output");
    setNodeType(SURFACEOUTPUT);
    setSockets(in,  IN);
    setSockets(out,  OUT);
    setDynamicSocketsNode(IN);
}

void Node::setdisplacementOutput()
{
    V_NSockets in, out;
    V_NSocket P, N;
    P.type = POINT;
    P.name = "P";
    N.type = NORMAL;
    N.name = "N";
    in.append(P);
    in.append(N);
    setNodeName("Displacement Output");
    setNodeType(DISPLACEMENTOUTPUT);
    setSockets(in,  IN);
    setSockets(out,  OUT);
    setDynamicSocketsNode(IN);
}

void Node::setvolumeOutput()
{
    V_NSockets in, out;
    V_NSocket Ci, Oi;
    Ci.type = COLOR;
    Ci.name = "Ci";
    Oi.type = COLOR;
    Oi.name = "Oi";
    in.append(Ci);
    in.append(Oi);
    setNodeName("Volume Output");
    setNodeType(VOLUMEOUTPUT);
    setSockets(in,  IN);
    setSockets(out,  OUT);
    setDynamicSocketsNode(IN);
}

void Node::setlightOutput()
{
    V_NSockets in, out;
    V_NSocket Cl;
    Cl.type = COLOR;
    Cl.name = "Cl";
    in.append(Cl);
    setNodeName("Light Output");
    setNodeType(LIGHTOUTPUT);
    setSockets(in,  IN);
    setSockets(out,  OUT);
    setDynamicSocketsNode(IN);
}

void Node::setDynamicSocketsNode(socket_dir dir, socket_type t)
{
    V_NSocket varsocketdata;
    varsocketdata.type = t;
    varsocketdata.name = "Add Socket";
    varsocketdata.cntdSockets.clear();
    varsocket = new NSocket(varsocketdata);
    varsocket->isVariable = true;
    addSocket(varsocket, dir);
}

bool Node::isInput(Node *node)
{
    if(node->NodeType == SURFACEINPUT
       ||node->NodeType == DISPLACEMENTINPUT
       ||node->NodeType == VOLUMEINPUT
       ||node->NodeType == LIGHTINPUT)
        return true;
    else
        return false;
}

FunctionNode::FunctionNode()
{
    initNode();
}

    ContainerNode::ContainerNode(bool raw)
{
    setNodeType(CONTAINER);
    initNode();
    if(!raw)
    {
        SocketNode *inNode = new SocketNode(IN, this);
        SocketNode *outNode = new SocketNode(OUT, this);
    }
}

ContainerNode::ContainerNode(QString name, bool raw)
{
    setNodeName(name);
    initNode();
    SocketNode *inNode = new SocketNode(IN, this);
    SocketNode *outNode = new SocketNode(OUT, this);
}

void ContainerNode::addMappedSocket(NSocket *socket, socket_dir dir)
{
    addSocket(socket, dir);
    NSocket *mapped_socket = new NSocket(socket->Socket);
    if(dir == IN)
        inSocketNode->addSocket(mapped_socket, OUT);
    else
        outSocketNode->addSocket(mapped_socket, IN);

    socket_map.insert(socket, mapped_socket);     
}

void ContainerNode::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *addtolibaction = contextMenu->addAction("Add to Library");

    connect(addtolibaction, SIGNAL(triggered()), this, SLOT(addtolib()));
}

void ContainerNode::addtolib()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QString filename;
    filename.append("nodes/");
    filename.append(node_name->toPlainText());
    filename.append(".node");
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out<<this;
}

void ContainerNode::initNode()
{
    Node::initNode();
    if(NodeType == CONTAINER)
        createContextMenu();

    setContainerData(new Shader_Space);
}

Node* ContainerNode::copyNode(QHash<NSocket *, NSocket *> *socketmapping)
{
    ContainerNode *newNode = new ContainerNode(node_name->toPlainText());
    foreach(NSocket *socket, *N_inSockets)
    {
        NSocket *newsocket = new NSocket(socket->Socket);
        socketmapping->insert(socket, newsocket);
        socket->Socket.cntdSockets.clear();
        newNode->addSocket(newsocket, socket->Socket.dir);
    }

    foreach(NSocket *socket, *N_outSockets)
    {
        NSocket *newsocket = new NSocket(socket->Socket);
        socketmapping->insert(socket, newsocket);
        newsocket->Socket.cntdSockets.clear();
        newNode->addSocket(newsocket, socket->Socket.dir);
    }
    newNode->setNodeType(NodeType);
    newNode->setContainerData(ContainerData);
    return newNode;
}

void ContainerNode::C_addItems(QList<Node *> nodes)
{
    foreach(Node *node, nodes)
    {
        Shader_Space *space = (Shader_Space*)ContainerData;
        space->addNode(node);
    }
}

void ContainerNode::setInputs(Node *inputNode)
{
    inSocketNode = inputNode;
    Shader_Space *space = (Shader_Space*)ContainerData;
    space->addNode(inputNode);
    SocketNode *snode = (SocketNode*)inputNode;
}

void ContainerNode::setOutputs(Node *outputNode)
{
    outSocketNode = outputNode;
    Shader_Space *space = (Shader_Space*)ContainerData;
    space->addNode(outputNode);
    SocketNode *snode = (SocketNode*)outputNode;
}

void ContainerNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!ContainerData)
        return;
    Shader_Space *space = (Shader_Space*)scene();
    space->changeScene(ContainerData);
}

void ContainerNode::newSocket(NSocket *socket)
{
    NSocket *newsocket = new NSocket(socket->Socket);
    newsocket->isVariable = false;
    socket_map.insert(socket, newsocket);
    socket_dir dir;
    if (newsocket->Socket.dir == IN) dir = OUT;
    else dir = IN;
    addSocket(newsocket, dir);
}

void ContainerNode::killSocket(NSocket *socket)
{
    NSocket *contsocket = socket_map[socket];
    socket_map.remove(socket);
    removeSocket(contsocket);
}

void ContainerNode::setSocketVarName(NSocket *socket)
{
    if(N_inSockets->isEmpty())
    {
        socket->Socket.varname = socket->Socket.name.toLower().replace(" ", "_");
        socket->setToolTip(socket->Socket.varname);
    }
    else
        Node::setSocketVarName(socket);
}

void ContainerNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(NodeType == CONTAINER)
        contextMenu->exec(event->screenPos());
}

NSocket *ContainerNode::getMappedSNSocket(NSocket *socket)
{
    return socket_map.value(socket);
}

NSocket *ContainerNode::getMappedCntNSocket(NSocket *socket)
{
    return socket_map.key(socket);
}

ConditionContainerNode::ConditionContainerNode()
{
    initNode();
    V_NSocket in;
    in.type = CONDITION;
    in.name = "Condition";
    SocketNode *inNode = new SocketNode(IN, this);
    SocketNode *outNode = new SocketNode(OUT, this);
    addMappedSocket(new NSocket(in), IN);
}

SocketNode::SocketNode(socket_dir dir, ContainerNode *contnode)
{
    initNode();
    isLoopSocket = false;
    if(contnode == 0)
        return;
    if (dir == IN)
        setInSocketNode(contnode);
    else
        setOutSocketNode(contnode);
}

bool SocketNode::loopSocket()
{
    return isLoopSocket;
}

void SocketNode::setLoopSocket(bool isLoop)
{
    isLoopSocket = isLoop;
}

void SocketNode::setInSocketNode(ContainerNode *contnode)
{
    setNodeType(INSOCKETS);

    setDynamicSocketsNode(OUT);
    setNodeName("Input");
    contnode->setInputs(this);

    connectToContainer(contnode);
}

void SocketNode::setOutSocketNode(ContainerNode *contnode)
{
    setNodeType(OUTSOCKETS);
    setDynamicSocketsNode(IN);
    setNodeName("Output");
    contnode->setOutputs(this);
    connectToContainer(contnode);
}

void SocketNode::connectToContainer(ContainerNode *contnode)
{
    connect(this, SIGNAL(socket_added(NSocket*)), contnode, SLOT(newSocket(NSocket*)));
    connect(this, SIGNAL(socket_removed(NSocket*)), contnode, SLOT(killSocket(NSocket*)));
}

void SocketNode::inc_var_socket()
{
    Node::inc_var_socket();
    emit socket_added(lastsocket);
}

void SocketNode::dec_var_socket(NSocket *socket)
{
    Node::dec_var_socket(socket);
    emit socket_removed(socket);
}

void SocketNode::add_socket(NSocket *socket)
{
    inc_var_socket();
}

void SocketNode::remove_socket(NSocket *socket)
{
    dec_var_socket(socket);
}

NSocket *SocketNode::getLastSocket()
{
    return lastsocket;
}


LoopSocketNode::LoopSocketNode(socket_dir dir, ContainerNode *contnode)
    : SocketNode(dir, contnode)
{
    isLoopSocket = true;
    partner = 0;
}

void LoopSocketNode::dec_var_socket(NSocket *socket)
{
    SocketNode::dec_var_socket(socket);
    if(partner != 0)
    {
        deletePartnerSocket(socket);
    }
}

void LoopSocketNode::createPartnerSocket(NSocket *socket)
{
    NSocket *partnerSocket = new NSocket(socket->Socket);
    partner->addSocket(partnerSocket, OUT);
    loopSocketMap.insert(socket, partnerSocket);
}

void LoopSocketNode::deletePartnerSocket(NSocket *socket)
{
    NSocket *partnerSocket = getPartnerSocket(socket);
    partner->removeSocket(partnerSocket);
    loopSocketMap.remove(socket);
}

void LoopSocketNode::setPartner(LoopSocketNode *p)
{
    partner = p;
}

NSocket *LoopSocketNode::getPartnerSocket(NSocket *socket)
{
    return loopSocketMap.value(socket);
}

void LoopSocketNode::addSocket(NSocket *socket, socket_dir dir)
{
    Node::addSocket(socket, dir);
}

void LoopSocketNode::inc_var_socket()
{
    SocketNode::inc_var_socket();
    if(partner) createPartnerSocket(lastsocket);
}

ConditionNode::ConditionNode(NType t)
{
    setNodeType(t);
    initNode();
}

void ConditionNode::initNode()
{
    Node::initNode();
    V_NSocket out;
    out.type = CONDITION;
    out.name = "Output";
    addSocket(new NSocket(out), OUT);
    if(NodeType != AND && NodeType != OR)
    {
        V_NSocket in;
        in.type = NodeType == NOT ? CONDITION : VARIABLE; //TODO: WTF?!?!
        in.name = "Input";
        addSocket(new NSocket(in), IN);
        if(NodeType != NOT)
        {
            V_NSocket in2;
            in2.type = VARIABLE;
            in2.name = "Input";
            addSocket(new NSocket(in2), IN);
        }
    }
    switch(NodeType)
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
    }
}

MathNode::MathNode(NType t)
{
    initNode();
    setNodeType(t);
    setDynamicSocketsNode(IN);
    switch(t)
    {
    case ADD:
        setNodeName("Add");
        break;
    case SUBTRACT:
        setNodeName("Subtract");
        break;
    case MULTIPLY:
        setNodeName("Multiply");
        break;
    case DIVIDE:
        setNodeName("Divide");
        break;
    case DOTPRODUCT:
        setNodeName("Dot Product");
        break;
    }

    V_NSocket resultd;
    resultd.name = "Result";
    resultd.type = VARIABLE;
    NSocket *result = new NSocket(resultd);
    addSocket(result, OUT);
}

void MathNode::inc_var_socket()
{
    Node::inc_var_socket();
//    varsocket->Socket.type = lastsocket->Socket.type;
    N_outSockets->first()->Socket.type = N_inSockets->first()->Socket.type;
}

void MathNode::dec_var_socket(NSocket *socket)
{
    Node::dec_var_socket(socket);
    NSocket *outsocket = N_outSockets->first();
    if(varcnt == 0 && outsocket->Socket.links.size() == 0)
    {
        outsocket->Socket.type = VARIABLE;
        varsocket->Socket.type = VARIABLE;
    }
    update();
}

void MathNode::setSocketType(socket_type t, QList<Node *> *checked_nodes)
{
    bool first_lvl;
    if(checked_nodes == 0)
    {
        first_lvl = true;
        checked_nodes = new QList<Node*>;
    }
    Node *node;

    foreach(NSocket *socket, *N_inSockets)
    {
        socket->Socket.type = t;
        foreach(V_NSocket *sdata, socket->Socket.cntdSockets)
        {
            node = (Node*)sdata->node;
            if(isMathNode(node) && !checked_nodes->contains(node))
            {
                checked_nodes->append(node);
                MathNode *mnode = (MathNode*)node;
                mnode->setSocketType(t, checked_nodes);
            }
        }
    }
    foreach(NSocket *socket, *N_outSockets)
    {
        socket->Socket.type = t;
        foreach(V_NSocket *sdata, socket->Socket.cntdSockets)
        {
            node = (Node*)sdata->node;
            if(isMathNode(node) && !checked_nodes->contains(node))
            {
                checked_nodes->append(node);
                MathNode *mnode = (MathNode*)node;
                mnode->setSocketType(t, checked_nodes);
            }
        }
    }
    update();
    if(first_lvl) delete checked_nodes;
}

bool Node::isMathNode(Node *node)
{
    return node->NodeType == ADD
       || node->NodeType == SUBTRACT
       || node->NodeType == MULTIPLY
       || node->NodeType == DIVIDE;
}

void ValueNode::initNode()
{
    Node::initNode();
    proxy = new QGraphicsProxyWidget;
    proxy->setParentItem(this);
    int width = NodeWidth();
    int height = NodeHeight(1);

    isShaderInput = false;
    createContextMenu();
}

void ValueNode::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *shaderinputAction = contextMenu->addAction("Shader Parameter");
    shaderinputAction->setCheckable(true);
    connect(shaderinputAction, SIGNAL(toggled(bool)), this, SLOT(setShaderInput(bool)));
}

void ValueNode::setShaderInput(bool isInput)
{
    isShaderInput = isInput;
}

void ValueNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}


void ValueNode::setValueEditor(QWidget *editor)
{
    widget = editor;
    proxy->setWidget(widget);
    widget->resize(NodeWidth() - 8, 25);
    proxy->setPos(QPointF(4-NodeWidth()/2, 8));
}

int ValueNode::NodeWidth() const
{
    int node_width = 150;
    return node_width;
};

int ValueNode::NodeHeight(int numSockets) const
{
    int node_height = 60 + (17*numSockets);
    return node_height;
};

ColorButton::ColorButton()
{
    setFlat(true);
    connect(this, SIGNAL(clicked()), this, SLOT(setColor()));
}

void ColorButton::setColor()
{
    QColor newcolor = QColorDialog::getColor();
    setPalette(QPalette(newcolor));
    emit clicked(newcolor);
    update();
}

ColorValueNode::ColorValueNode()
{
    initNode();
    setNodeType(COLORNODE);
    setNodeName("Color Value");

    ColorButton *cbutton = new ColorButton;
    connect(cbutton, SIGNAL(clicked(QColor)), this, SLOT(setValue(QColor)));
    setValueEditor(cbutton);

    V_NSocket sdata;
    sdata.type = COLOR;
    sdata.name = "Color";
    NSocket *out = new NSocket(sdata);
    addSocket(out, OUT);
}

void ColorValueNode::setValue(QColor newvalue)
{
    colorvalue = newvalue;
}

StringValueNode::StringValueNode()
{
    initNode();
    setNodeType(STRINGNODE);
    setNodeName("String Value");
    QLineEdit *lineedit = new QLineEdit;
    connect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(setValue(QString)));
    setValueEditor(lineedit);

    V_NSocket sdata;
    sdata.type = STRING;
    sdata.name = "String";
    NSocket *out = new NSocket(sdata);
    addSocket(out, OUT);
}

void StringValueNode::setValue(QString newstring)
{
    stringvalue = newstring;
}

FloatValueNode::FloatValueNode()
{
    initNode();
    setNodeType(FLOATNODE);
    setNodeName("Float Value");
    QDoubleSpinBox *spinbox = new QDoubleSpinBox;
    spinbox->setRange(-1000, 1000);
    connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    setValueEditor(spinbox);

    V_NSocket sdata;
    sdata.type = FLOAT;
    sdata.name = "Float";
    NSocket *out = new NSocket(sdata);
    addSocket(out, OUT);
}
void FloatValueNode::setValue(double newval)
{
    floatvalue = newval;
}

VectorValueNode::VectorValueNode()
{
    initNode();
    setNodeType(VECTORNODE);
    setNodeName("Vector Value");
}

void VectorValueNode::setValue()
{

}

WhileNode::WhileNode(bool raw)
{
    setNodeType(WHILE);
    LoopNode::initNode(raw);
    if(!raw)
    {
        V_NSocket cond;
        cond.type = CONDITION;
        cond.name = "Condition";
        NSocket *condition = new NSocket(cond);
        outSocketNode->addSocket(condition, IN);
    }
}

ForNode::ForNode(bool raw)
{
    setNodeType(FOR);
    LoopNode::initNode(raw);
    if(!raw)
    {
        V_NSocket start, end, step;
        start.type = FLOAT;
        start.name = "Start";
        end.type = FLOAT;
        end.name = "End";
        step.type = FLOAT;
        step.name = "Step";
        addMappedSocket(new NSocket(start), IN);
        addMappedSocket(new NSocket(end), IN);
        addMappedSocket(new NSocket(step), IN);
    }
}

IlluminanceNode::IlluminanceNode(bool raw)
{
    setNodeType(ILLUMINANCE);
    LoopNode::initNode(raw);
    if(!raw)
    {
        V_NSocket point, direction, angle, category, mpassing;
        category.type = STRING;
        category.name = "Category";
        category.isToken = false;
        point.type = POINT;
        point.name = "Point";
        point.isToken = false;
        direction.type = VECTOR;
        direction.name = "Direction";
        direction.isToken = false;
        angle.type = FLOAT;
        angle.name = "Angle";
        angle.isToken = false;
        mpassing.name = "Message Passing";
        mpassing.isToken = false;
        mpassing.type = STRING;

        NSocket *MPassing = new NSocket(mpassing);
        NSocket *Category = new NSocket(category);
        NSocket *Point = new NSocket(point);
        NSocket *Direction = new NSocket(direction);
        NSocket *Angle = new NSocket(angle);

        addMappedSocket(Category, IN);
        addMappedSocket(Point, IN);
        addMappedSocket(Direction, IN);
        addMappedSocket(Angle, IN);
        addMappedSocket(MPassing, IN);
    }
}

IlluminateNode::IlluminateNode(bool raw)
{
    setNodeType(ILLUMINATE);
    LoopNode::initNode(raw);
    if(!raw)
    {
        V_NSocket point, direction, angle;
        point.type = POINT;
        point.name = "Point";
        point.isToken = false;
        direction.type = VECTOR;
        direction.name = "Direction";
        direction.isToken = false;
        angle.type = FLOAT;
        angle.name = "Angle";
        angle.isToken = false;

        NSocket *Point = new NSocket(point);
        NSocket *Direction = new NSocket(direction);
        NSocket *Angle = new NSocket(angle);

        addMappedSocket(Point, IN);
        addMappedSocket(Direction, IN);
        addMappedSocket(Angle, IN);
    }
}

GatherNode::GatherNode(bool raw)
{
    setNodeType(GATHER);
    LoopNode::initNode(raw);
    if(!raw)
    {
        V_NSocket point, direction, angle, category, mpassing, samples;
        category.type = STRING;
        category.name = "Category";
        category.isToken = false;
        point.type = POINT;
        point.name = "Point";
        point.isToken = false;
        direction.type = VECTOR;
        direction.name = "Direction";
        direction.isToken = false;
        angle.type = FLOAT;
        angle.name = "Angle";
        angle.isToken = false;
        mpassing.name = "Message Passing";
        mpassing.isToken = false;
        mpassing.type = STRING;
        samples.name = "Samples";
        samples.type = FLOAT;
        samples.isToken = false;

        NSocket *MPassing = new NSocket(mpassing);
        NSocket *Category = new NSocket(category);
        NSocket *Point = new NSocket(point);
        NSocket *Direction = new NSocket(direction);
        NSocket *Angle = new NSocket(angle);
        NSocket *Samples = new NSocket(samples);

        addMappedSocket(Category, IN);
        addMappedSocket(Point, IN);
        addMappedSocket(Direction, IN);
        addMappedSocket(Angle, IN);
        addMappedSocket(MPassing, IN);
        addMappedSocket(Samples, IN);
    }
}

SolarNode::SolarNode(bool raw)
{
    setNodeType(SOLAR);
    LoopNode::initNode(raw);
    if(!raw)
    {
        V_NSocket axis, angle;
        axis.type = VECTOR;
        axis.name = "Axis";
        axis.isToken = false;
        angle.type = FLOAT;
        angle.name = "Angle";
        angle.isToken = false;

        NSocket *Axis = new NSocket(axis);
        NSocket *Angle = new NSocket(angle);

        addMappedSocket(Axis, IN);
        addMappedSocket(Angle, IN);
    }
}

void LoopNode::initNode(bool raw)
{
    Node::initNode();
    setContainerData(new Shader_Space);
    if(!raw)
    {
        LoopSocketNode *loutNode, *linNode;

        linNode = new LoopSocketNode(IN, this);
        loutNode = new LoopSocketNode(OUT, this);
        loutNode->setPartner(linNode);
    }
}

bool LoopNode::isLoopNode(Node *node)
{
    return (node->NodeType == FOR
            ||node->NodeType == WHILE
            ||node->NodeType == ILLUMINANCE
            ||node->NodeType == ILLUMINATE
            ||node->NodeType == SOLAR
            ||node->NodeType == GATHER);
}

OutputNode::OutputNode()
{
    initNode();
    createMenu();
    filename = "";
}

void OutputNode::createMenu()
{
    contextMenu = new QMenu;
    QAction *nameAction = contextMenu->addAction("set Shader Name");
    QAction *writecodeAction = contextMenu->addAction("Create Code");
    QAction *compileAction = contextMenu->addAction("write and compile code");

    connect(writecodeAction, SIGNAL(triggered()), this, SLOT(writeCode()));
    connect(nameAction, SIGNAL(triggered()), this, SLOT(changeName()));
}

void OutputNode::writeCode()
{
    if(filename=="")
        filename = QFileDialog::getSaveFileName();

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    ShaderWriter sw(this);
    QString code = sw.getCode();
    stream<<code;
    file.close();
}

void OutputNode::changeName()
{
    bool ok;
    QString shadertype, newnodename;
    switch(NodeType)
    {
    case SURFACEOUTPUT:
        shadertype = "Surface Output";
        break;
    case DISPLACEMENTOUTPUT:
        shadertype = "Displacement Output";
        break;
    case VOLUMEOUTPUT:
        shadertype = "Volume Output";
        break;
    case LIGHTOUTPUT:
        shadertype = "Light Output";
        break;
    }

    QString newname = QInputDialog::getText(scene()->views().first(), "Shader Name", "Name", QLineEdit::Normal, "", &ok);
    if(ok)
    {
        ShaderName = newname;
        newnodename = shadertype;
        newnodename += " (";
        newnodename += ShaderName;
        newnodename += " )";
        node_name->setPlainText(newnodename);
    }
}

void OutputNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}

void OutputNode::setSocketVarName(NSocket *socket)
{
    if(!socket->isVariable)
    {
        socket->Socket.varname = socket->Socket.name;
        socket->setToolTip(socket->Socket.name);
    }
    else
        Node::setSocketVarName(socket);
}

InputNode::InputNode()
{
    initNode();
}

void InputNode::setSocketVarName(NSocket *socket)
{
    socket->setToolTip(socket->Socket.varname);
    socket->Socket.varname = socket->Socket.name;
}
