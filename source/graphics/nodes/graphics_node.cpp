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
#include "graphics_node.h"

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

#include "source/data/base/frg.h"
#include "source/data/base/frg_shader_author.h"
#include "source/data/base/dnspace.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/base/project.h"
#include "source/data/callbacks.h"

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
    VNode *vnode = static_cast<VNode*>(parentItem());
    DNode *pNode = vnode->data;
    pNode->setNodeName(toPlainText());
    if((pNode->isContainer()))
    {
        ContainerNode *cNode = static_cast<ContainerNode*>(pNode);
        if(cNode->getContainerData())
        {
            cNode->getContainerData()->setName(toPlainText());
        }
    }
    if(scene())
		FRG::Space->leaveEditNameMode();

    setTextInteractionFlags(Qt::NoTextInteraction);
    textCursor().clearSelection();
}

void NodeName::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(scene())
    {
        VNSpace *space = (VNSpace*)scene();
        space->enterEditNameMode();
    }
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

VNode::VNode(DNode *data)
    : data(data), node_name(new NodeName("", this)), node_width(30), node_height(30)
{
    this->data = data;
    node_name = new NodeName("", this);
	setCacheMode(ItemCoordinateCache);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);

    node_name->setDefaultTextColor(QColor(255, 255, 255));

    //QGraphicsDropShadowEffect *dropshad = new QGraphicsDropShadowEffect;
    //dropshad->setBlurRadius(10);
    //dropshad->setOffset(5);
    //dropshad->setColor(QColor(10, 10, 10, 200));
    //setGraphicsEffect(dropshad);

	foreach(DinSocket *socket, data->getInSockets())
		socket->createSocketVis(this);

	foreach(DoutSocket *socket, data->getOutSockets())
		socket->createSocketVis(this);

	cacheSocketSize();
    updateNodeVis();

    data->regAddSocketCB(new VNodeUpdateCallback(this));
	//setPos(FRG::CurrentProject->getNodePosition(data));
}

VNode::~VNode()
{
}

int VNode::getNodeWidth()
{
	return node_width;
}

void VNode::setNodeWidth(int w)
{
	node_width = w;
}

int VNode::getNodeHeight()
{
	return node_height;
}

void VNode::setNodeHeight(int h)
{
	node_height = h;
}

void VNode::cacheSocketSize()
{
	if(!data->getInSockets().isEmpty())
		socket_size = data->getInSockets().first()->getSocketVis()->getHeight();
    else if(!data->getOutSockets().isEmpty())
		socket_size = data->getOutSockets().first()->getSocketVis()->getHeight();
}


void VNode::drawName()
{
    if(FRG::Space->isEditNameMode()) return;
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
	foreach(DSocket *socket, data->getInSockets())
        if((socket->getSocketVis())
            &&socket->getSocketVis()->getSocketNameVisWidth() > node_width)
                node_width = socket->getSocketVis()->getSocketNameVisWidth();

	foreach(DSocket *socket, data->getOutSockets())
        if((socket->getSocketVis())
            &&socket->getSocketVis()->getSocketNameVisWidth() > node_width)
            node_width = socket->getSocketVis()->getSocketNameVisWidth();

	node_width += 6 + socket_size; //Left/Right border margin
};

void VNode::NodeHeight(int numSockets)
{
    if(!numSockets)
        node_height = 25;
    else
        node_height = (2 + socket_size) * numSockets + 6;
};

QRectF VNode::boundingRect() const
{
    return QRectF(-node_width/2, -node_height/2, node_width, node_height);
};

void VNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor nodeBGColor;
    QColor nodeHColor;
    QColor textColor;
    QPen node_outline;
    int nodePen = 4;

    if (isSelected())
    {
        nodeBGColor = QColor(100, 100, 100);
        textColor = QColor(255, 255, 255);
        node_outline.setColor(QColor(255, 145, 0, 100));
        node_outline.setWidth(nodePen);
    }
    else
    {
        nodeBGColor = QColor(100, 100, 100);
        textColor = QColor(255, 255, 255, 255);
        node_outline.setColor(QColor(80, 80, 80, 100));
        node_outline.setWidth(nodePen);
    };
    painter->setPen(node_outline);
    painter->setBrush(QBrush(nodeBGColor, Qt::SolidPattern));
    painter->drawRect(-node_width/2, -node_height/2, node_width, node_height);
    drawName();
};

void VNode::recalcNodeVis()
{
    int space=2;
    int socketPos = 2*space - (node_height/2);

	foreach(DinSocket *insocket, data->getInSockets())
    {
        insocket->getSocketVis()->setPos((-node_width/2) + (4 + socket_size/2), socketPos+(socket_size/2));
        socketPos += socket_size + space;
	}

    foreach(DoutSocket *out, data->getOutSockets())
    {
        out->getSocketVis()->setPos((node_width/2) - (4 + socket_size/2), socketPos+(socket_size/2));
        socketPos += socket_size + space;
	}
}

void VNode::updateNodeVis()    
{
    NodeHeight(data->getInSockets().size() + data->getOutSockets().size());
    drawName();
	NodeWidth();

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
    FRG::Author->moveIntoSpace(container->getContainerData());
}

void VContainerNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(data->getNodeType() == CONTAINER)
        contextMenu->exec(event->screenPos());
}

VValueNode::VValueNode(DNode *data)
    :VNode(data)
{
    proxy = new QGraphicsProxyWidget;
    proxy->setParentItem(this);
    createContextMenu();
}

VValueNode::~VValueNode()
{
    delete contextMenu;
    delete widget;
}


void VValueNode::setShaderInput(bool tgl)
{
    ValueNode* vdata = (ValueNode*)data;
    vdata->setShaderInput(tgl);
}

void VValueNode::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *shaderinputAction =  contextMenu->addAction("Shader Parameter");
    shaderinputAction->setCheckable(true);
    connect(shaderinputAction, SIGNAL(toggled(bool)), this, SLOT(setShaderInput(bool)));
}

void VValueNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}


void VValueNode::setValueEditor(QWidget *editor)
{
    widget = editor;
    proxy->setWidget(widget);
    widget->resize(getNodeWidth() - 8, 25);
    proxy->setPos(QPointF(4-getNodeWidth()/2, 8));
}

void VValueNode::NodeWidth()
{
    setNodeWidth(20);
};

void VValueNode::NodeHeight(int numSockets)
{
    setNodeHeight( 60 + (17*numSockets));
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

VColorValueNode::VColorValueNode(DNode *data)
    : VValueNode(data)
{
    ColorButton *cbutton = new ColorButton;
    contextMenu->connect(cbutton, SIGNAL(clicked(QColor)), this, SLOT(setValue(QColor)));
    setValueEditor(cbutton);
}

void VColorValueNode::setValue(QColor color)
{
    ColorValueNode* data = (ColorValueNode*)this->data;
    data->setValue(color);
}

VStringValueNode::VStringValueNode(DNode *data)
    :VValueNode(data)
{
    QLineEdit *lineedit = new QLineEdit;
    contextMenu->connect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(setValue(QString)));
    setValueEditor(lineedit);
}

void VStringValueNode::setValue(QString string)
{
    StringValueNode* data = (StringValueNode*)this->data;
    data->setValue(string);
}

VFloatValueNode::VFloatValueNode(DNode *data)
    :VValueNode(data)
{
    QDoubleSpinBox *spinbox = new QDoubleSpinBox;
    spinbox->setRange(-1000, 1000);
    contextMenu->connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    setValueEditor(spinbox);
}

void VFloatValueNode::setValue(double fval)
{
    FloatValueNode *data = (FloatValueNode*)this->data;
    data->setValue(fval);
}

VVectorValueNode::VVectorValueNode(DNode *data)
    :VValueNode(data)
{
}

void VVectorValueNode::setValue()
{

}

VOutputNode::VOutputNode(DNode *data)
    :VNode(data)
{
    this->data = data;
    createMenu();
}

void VOutputNode::createMenu()
{
    contextMenu = new QMenu;
    QAction *nameAction = contextMenu->addAction("set Shader Name");
    QAction *writecodeAction = contextMenu->addAction("Create Code");
    QAction *compileAction = contextMenu->addAction("write and compile code");

    connect(writecodeAction, SIGNAL(triggered()), this, SLOT(writeCode()));
    connect(nameAction, SIGNAL(triggered()), this, SLOT(changeName()));
}

void VOutputNode::changeName()
{
    OutputNode *data = (OutputNode*)this->data;
    bool ok;
    QString newname = QInputDialog::getText(scene()->views().first(), "Shader Name", "Name", QLineEdit::Normal, "", &ok);
    if(ok)
        data->changeName(newname);
}

void VOutputNode::writeCode()
{
    OutputNode *data = (OutputNode*)this->data;
    if(data->getFileName() == "")
        data->setFileName(QFileDialog::getSaveFileName(0, QString(), QString(), "RSL Code File (*.sl)"));
    data->writeCode();
}

void VOutputNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}
