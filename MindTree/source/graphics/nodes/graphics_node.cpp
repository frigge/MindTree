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
#include "QGraphicsSceneMouseEvent"
#include "QGraphicsEffect"
#include "QLabel"
#include  "QLayout"
#include "QColorDialog"
#include "QDoubleSpinBox"
#include "QInputDialog"
#include "QCoreApplication"
#include "QFileDialog"
#include "QTextStream"
#include "QTextCursor"
#include "QGraphicsProxyWidget"

#include "math.h"

#include "source/lib/mindtree_core.h"
//#include "source/data/base/dnspace.h"
//#include "source/data/base/project.h"
#include "data/callbacks.h"
#include "graphics/prop_vis.h"
//#include "source/data/code_generator/outputs.h"
//#include "source/graphics/shader_view.h"
#include "source/graphics/base/vnspace.h"
#include "source/graphics/nodelib.h"
#include "graphics_node.h"

using namespace MindTree;

AbstractNodeGraphics::AbstractNodeGraphics(DNode_ptr data)
    : data(data)
{
}

AbstractNodeGraphics::~AbstractNodeGraphics()
{
}

PyNodeGraphics::PyNodeGraphics(DNode* data)
    : AbstractNodeGraphics(std::make_shared<DNode>(data))
{
}

PyNodeGraphics::~PyNodeGraphics()
{
}

QRectF PyNodeGraphics::boundingRect()const
{
}

void PyNodeGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)    
{
}

ChooseInputAction::ChooseInputAction(DSocket *socket, VNode *node)
    : QAction(socket->getName(), 0), socket(socket), vnode(node)
{
    connect(this, SIGNAL(triggered()), this, SLOT(addLink()));
}

void ChooseInputAction::addLink()
{
    VNSocket *vis = new VNSocket(socket, vnode);
    VNSpace *space = (VNSpace*)vnode->scene();
    space->addLink(vis);
}

NodeName::NodeName(QString name, QGraphicsItem *parent)
    : QGraphicsTextItem(name, parent)
{
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    setFlag(ItemIsFocusable, true);
}

void NodeName::focusOutEvent(QFocusEvent *event)
{
    QGraphicsTextItem::focusOutEvent(event);
    VNode *vnode = static_cast<VNode*>(parentItem());
    DNode *pNode = vnode->data;

    if(toPlainText() != "") {
        pNode->setNodeName(toPlainText());
        if((pNode->isContainer())) {
            ContainerNode *cNode = static_cast<ContainerNode*>(pNode);
            if(cNode->getContainerData())
                cNode->getContainerData()->setName(pNode->getNodeName());
        }
    }
    else setPlainText(pNode->getNodeName());
    //if(scene()) FRG::Space->leaveEditNameMode();
    setTextInteractionFlags(Qt::NoTextInteraction);
    textCursor().clearSelection();
    setPlainText(pNode->getNodeName());
}

void NodeName::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    //if(scene()) {
    //    VNSpace *space = (VNSpace*)scene();
    //    space->enterEditNameMode();
    //}
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

VNode::VNode(DNode *data)
    : data(data), 
    node_name(new NodeName("", this)), 
    node_width(30), 
    node_height(30), 
    nodeColor(50, 50, 50), 
    inpM(0), 
    moving(false)
{
    setZValue(2);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    //setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);
    setAcceptDrops(true);

    node_name->setDefaultTextColor(QColor(255, 255, 255));

    //QGraphicsDropShadowEffect *dropshad = new QGraphicsDropShadowEffect;
    //dropshad->setBlurRadius(4);
    //dropshad->setOffset(2);
    //dropshad->setColor(QColor(10, 10, 10, 200));
    //setGraphicsEffect(dropshad);

	foreach(DinSocket *socket, data->getInSockets())
        if(socket->getArray())
           new VNASocket((DAInSocket*)socket, this); 

	foreach(DoutSocket *socket, data->getOutSockets())
        new VNSocket(socket, this);

	cacheSocketSize();
    updateNodeVis();

    //cb = new VNodeUpdateCallback(this);
    //data->regAddSocketCB(cb);
}

VNSocket* VNode::getSocketVis(DSocket *socket)    
{
    foreach(QGraphicsItem *item, childItems())
        if(((VNSocket*)item)->getData() == socket)
            return (VNSocket*)item;
    return 0;
}

VNode::~VNode()
{
    //data->remAddSocketCB(cb);
}

int VNode::getNodeWidth()const
{
	return node_width;
}

void VNode::setNodeWidth(int w)
{
	node_width = w;
}

int VNode::getNodeHeight()const
{
	return node_height;
}

void VNode::setNodeHeight(int h)
{
	node_height = h;
}

void VNode::cacheSocketSize()
{
    socket_size = SOCKET_HEIGHT;
}

void VNode::dragEnterEvent(QGraphicsSceneDragDropEvent *event)    
{
    if(event->mimeData()->hasFormat("FRGShaderAuthor/OutSocket"))
        event->acceptProposedAction();
}

void VNode::dragMoveEvent(QGraphicsSceneDragDropEvent *event)    
{
    if(event->mimeData()->hasFormat("FRGShaderAuthor/OutSocket"))
        event->acceptProposedAction();
}

void VNode::dropEvent(QGraphicsSceneDragDropEvent *event)    
{
    QString format = event->mimeData()->formats().first();
    //if(!FRG::Space->isLinkNodeMode())
    //    return;
    if(event->mimeData()->hasFormat("FRGShaderAuthor/OutSocket")){
        QByteArray itemData;
        itemData = event->mimeData()->data("FRGShaderAuthor/OutSocket");

        QDataStream stream(&itemData, QIODevice::ReadOnly);
        int t;
        stream >> t;
        createInputMenu((socket_type)t, event->screenPos());
    }
    //else
    //    FRG::Space->cancelLinkNodeMode();
}

void VNode::createInputMenu(socket_type t, QPoint pos)    
{
    if(inpM) delete inpM;
    inpM = new QMenu;
    QList<DSocket*> compSockets;
    VNSpace *space = (VNSpace*)scene();
    foreach(DinSocket *socket, data->getInSockets())
        if(DSocket::isCompatible(socket->getType(), t))
            compSockets.append(socket);
    if(compSockets.size() > 1) {
        foreach(DSocket *socket, compSockets)
            inpM->addAction(new ChooseInputAction(socket, this));
        inpM->exec(pos);
    }
    else if(!compSockets.isEmpty()) {
        DSocket *socket = compSockets.first();
        
        VNSocket *vis = new VNSocket(socket, this);
        space->addLink(vis);
    }
    else
        space->cancelLinkNodeMode();
}

void VNode::drawName()
{
    //if(FRG::Space->isEditNameMode()) return;
    node_name->setPlainText(data->getNodeName());
    node_name->setPos(0-(node_width/2)+4, -(node_height/2)-20);
}

void VNode::NodeWidth()
{
    if(data->getInSockets().isEmpty()
        &&data->getOutSockets().isEmpty())
    {
        node_width = 25;
        return;
    }

    //fit the widest socket name
    node_width = 0;
    int border_margin = 6 + SOCKET_WIDTH;
	foreach(DSocket *socket, data->getInSockets()) {
        if(getSocketVis(socket))getSocketVis(socket)->updateNameVis();
        if((getSocketVis(socket))
            &&getSocketVis(socket)->getSocketNameVisWidth() > node_width)
            node_width = getSocketVis(socket)->getSocketNameVisWidth();
    }

	foreach(DSocket *socket, data->getOutSockets()) {
        if(getSocketVis(socket))getSocketVis(socket)->updateNameVis();
        if((getSocketVis(socket))
            &&getSocketVis(socket)->getSocketNameVisWidth() > node_width)
        {
            getSocketVis(socket)->updateNameVis();
            node_width = getSocketVis(socket)->getSocketNameVisWidth();
        }
    }

	node_width += border_margin;

    update(boundingRect());
};

void VNode::NodeHeight(int numSockets)
{
    if(!numSockets)
        node_height = 25;
    else {
        node_height = 6;
        foreach(DinSocket *socket, data->getInSockets())
            if(getSocketVis(socket))
                node_height += 2 + socket_size;
        foreach(DoutSocket *socket, data->getOutSockets())
                node_height += 2 + socket_size;
    }
};

QRectF VNode::boundingRect() const
{
    return QRectF(-node_width/2, -node_height/2, node_width, node_height);
};

void VNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)    
{
    data->setPos(pos());
    moving = false;
    QGraphicsItem::mouseReleaseEvent(event);
}

void VNode::mousePressEvent(QGraphicsSceneMouseEvent *event)    
{
    //if(event->button() == Qt::LeftButton)
    //    mouseOffset = event->scenePos() - data->getPos();
    moving = true;
    QGraphicsItem::mousePressEvent(event);
}

void VNode::mouseMoveEvent(QGraphicsSceneMouseEvent *event)    
{
    //QPointF mouseDir = event->scenePos() - lastMousePos;
    //double mouseDirLen = sqrt(pow(mouseDir.x(), 2) + pow(mouseDir.y(), 2));
    //if(mouseDirLen == 0)
    //    return;
    //int sign = mouseDirLen/abs(mouseDirLen);
    //foreach(QGraphicsItem *item, collidingItems())
    //    if(item->type() == VNode::Type) {
    //        QPointF collidingSide = item->pos() - pos();
    //        double collidingSideLen = sqrt(pow(collidingSide.x(), 2) + pow(collidingSide.y(), 2));
    //        int csSign = collidingSideLen/abs(collidingSideLen);
    //        if(csSign == sign)
    //            QGraphicsItem::mouseMoveEvent(event);
    //        return;
    //    }
    //lastMousePos = event->scenePos();
    QGraphicsItem::mouseMoveEvent(event);
}

void VNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPointF dpos = data->getPos();
    if(!moving && pos() != dpos)setPos(dpos);
    updateNodeVis();
    painter->setRenderHint(QPainter::Antialiasing);
    //painter->setClipRect(option->exposedRect);
    QColor textColor;
    QPen node_outline;
    int nodePen = 1;
    QPointF diagVec = QPointF(-node_width/2, node_height) - QPointF(node_width, -node_height/2);
    float length = sqrt(pow(diagVec.x(), 2) + pow(diagVec.y(), 2));
    QPointF ndiagVec(-diagVec.y() / length, diagVec.x() / length);

    QLinearGradient outlineCol(ndiagVec , -1 * ndiagVec);
    outlineCol.setColorAt(0, QColor(150, 150, 150));
    outlineCol.setColorAt(1, QColor(0, 0, 0));

    if (isSelected())
    {
        textColor = QColor(255, 255, 255);
        node_outline.setColor(QColor(255, 145, 0, 200));
        node_outline.setWidth(nodePen);
    }
    else
    {
        textColor = QColor(255, 255, 255, 255);
        //node_outline.setColor(QColor(30, 30, 30, 255));
        node_outline = QPen(QBrush(outlineCol), 0.5);
        //node_outline.setWidth(nodePen);
    };
    painter->setPen(node_outline);
    painter->setBrush(QBrush(nodeColor, Qt::SolidPattern));
//    painter->drawRect(-node_width/2, -node_height/2, node_width, node_height);
    painter->drawRoundedRect(-node_width/2, -node_height/2, node_width, node_height, 3,3);
    drawName();
};

void VNode::setNodeColor(QColor col)    
{
    std::cout<<"Node Color changed" << std::endl;
    nodeColor = col;
}

QColor VNode::getNodeColor()    
{
    return nodeColor; 
}

void VNode::recalcNodeVis()
{
    int space=2;
    int socketPos = 2*space - (node_height/2);

	foreach(DinSocket *insocket, data->getInSockets()) {
        if(getSocketVis(insocket)){
            getSocketVis(insocket)->setPos((-node_width/2) , socketPos+(socket_size/2));
            socketPos += socket_size + space;
        }
	}

    foreach(DoutSocket *out, data->getOutSockets()) {
        if(getSocketVis(out)){
            getSocketVis(out)->setPos((node_width/2), socketPos+(socket_size/2));
            socketPos += socket_size + space;
        }
	}
}

void VNode::updateNodeVis()    
{
    NodeHeight(data->getInSockets().size() + data->getOutSockets().size());
	NodeWidth();
    drawName();

    recalcNodeVis();
}

VContainerNode::VContainerNode(DNode *data)
    :VNode(data), contextMenu(0)
{
    if(data->getNodeType() == CONTAINER)
        createContextMenu();
}

VContainerNode::~VContainerNode()
{
    if(contextMenu)delete contextMenu;
}


void VContainerNode::addToLib()
{
    ContainerNode *cdata = (ContainerNode*)data;
    cdata->addtolib();
    FRG::lib->update();
}

void VContainerNode::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *addtolibaction = contextMenu->addAction("add to library");
    contextMenu->addAction(addtolibaction);

    connect(addtolibaction, SIGNAL(triggered()), this, SLOT(addToLib()));
}

void VContainerNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    ContainerNode *container = (ContainerNode*)data;
    if(!container->getContainerData())
        return;
    //FRG::Author->moveIntoSpace(container->getContainerData());
}

void VContainerNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(data->getNodeType() == CONTAINER)
        contextMenu->exec(event->screenPos());
}
