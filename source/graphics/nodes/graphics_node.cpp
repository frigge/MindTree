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

#include "math.h"

#include "source/data/base/frg.h"
#include "source/data/base/frg_shader_author.h"
#include "source/data/base/dnspace.h"
#include "source/graphics/base/vnspace.h"
#include "source/data/base/project.h"
#include "source/data/callbacks.h"
#include "source/graphics/sourcedock.h"

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

    if(toPlainText() != "")
    {
        pNode->setNodeName(toPlainText());
        if((pNode->isContainer()))
        {
            ContainerNode *cNode = static_cast<ContainerNode*>(pNode);
            if(cNode->getContainerData())
            {
                cNode->getContainerData()->setName(toPlainText());
            }
        }
    }
    else
        setPlainText(pNode->getNodeName());

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
    : data(data), node_name(new NodeName("", this)), node_width(30), node_height(30), nodeColor(100, 100, 100)
{
    this->data = data;
    setZValue(1);
	//setCacheMode(ItemCoordinateCache, QSize(10, 10));
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);

    node_name->setDefaultTextColor(QColor(255, 255, 255));

    QGraphicsDropShadowEffect *dropshad = new QGraphicsDropShadowEffect;
    dropshad->setBlurRadius(4);
    dropshad->setOffset(2);
    dropshad->setColor(QColor(10, 10, 10, 200));
    setGraphicsEffect(dropshad);

	foreach(DinSocket *socket, data->getInSockets())
		socket->createSocketVis(this);

	foreach(DoutSocket *socket, data->getOutSockets())
		socket->createSocketVis(this);

	cacheSocketSize();
    updateNodeVis();

    cb = new VNodeUpdateCallback(this);
    data->regAddSocketCB(cb);
	//setPos(FRG::CurrentProject->getNodePosition(data));
}

VNode::~VNode()
{
    data->remAddSocketCB(cb);
    delete cb;
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
    socket_size = SOCKET_HEIGHT;
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
    {
        if(socket->getSocketVis())socket->getSocketVis()->updateNameVis();
        if((socket->getSocketVis())
            &&socket->getSocketVis()->getSocketNameVisWidth() > node_width)
            node_width = socket->getSocketVis()->getSocketNameVisWidth();
    }

	foreach(DSocket *socket, data->getOutSockets())
    {
        if(socket->getSocketVis())socket->getSocketVis()->updateNameVis();
        if((socket->getSocketVis())
            &&socket->getSocketVis()->getSocketNameVisWidth() > node_width)
            socket->getSocketVis()->updateNameVis();
            node_width = socket->getSocketVis()->getSocketNameVisWidth();
    }

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
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setClipRect(option->exposedRect);
    QColor textColor;
    QPen node_outline;
    int nodePen = 1;
    QPointF diagVec = QPointF(-node_width/2, node_height) - QPointF(node_width, -node_height/2);
    float length = sqrt(pow(diagVec.x(), 2) + pow(diagVec.y(), 2));
    QPointF ndiagVec(-diagVec.y() / length, diagVec.x() / length);

    QLinearGradient outlineCol(ndiagVec , -1 * ndiagVec);
    outlineCol.setColorAt(0, QColor(200, 200, 200));
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
    painter->drawRoundedRect(-node_width/2, -node_height/2, node_width, node_height, 5,5);
    drawName();
};

void VNode::setNodeColor(QColor col)    
{
    nodeColor = col;
}

void VNode::recalcNodeVis()
{
    int space=2;
    int socketPos = 2*space - (node_height/2);

	foreach(DinSocket *insocket, data->getInSockets())
    {
        if(!insocket->getSocketVis()) continue;
        //insocket->getSocketVis()->setPos((-node_width/2) + (4 + socket_size/2), socketPos+(socket_size/2));
        //if(insocket->isArray() && insocket->getID() != insocket->getArrayID())
        //{
        //    insocket->getSocketVis()->setPos(DSocket::getSocket(insocket->getArrayID())->getSocketVis()->pos());
        //    insocket->getSocketVis()->setVisible(false);
        //}
        //else
        {
            insocket->getSocketVis()->setPos((-node_width/2) , socketPos+(socket_size/2));
            socketPos += socket_size + space;
        }
	}

    foreach(DoutSocket *out, data->getOutSockets())
    {
        //out->getSocketVis()->setPos((node_width/2) - (4 + socket_size/2), socketPos+(socket_size/2));
        if(!out->getSocketVis()) continue;
        //if(out->isArray() && out->getID() != out->getArrayID())
        //{
        //    out->getSocketVis()->setPos(DSocket::getSocket(out->getArrayID())->getSocketVis()->pos());
        //    out->getSocketVis()->setVisible(false);
        //}
        //else
        {
            out->getSocketVis()->setPos((node_width/2), socketPos+(socket_size/2));
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
    FRG::Author->moveIntoSpace(container->getContainerData());
}

void VContainerNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(data->getNodeType() == CONTAINER)
        contextMenu->exec(event->screenPos());
}

VValueNode::VValueNode(DNode *data)
    :VNode(data), widget(0), proxy(new QGraphicsProxyWidget)
{
    proxy->setParentItem(this);
    createContextMenu();

    ValueNode *node = static_cast<ValueNode*>(data);
    if(node->isShaderInput())
        setNodeColor(QColor(255, 135, 0));
    else
        setNodeColor(QColor(100, 100, 100));

    data->getOutSockets().first()->getSocketVis()->setDrawName(false);
    data->getOutSockets().first()->getSocketVis()->setBlockContextMenu(true);
    updateNodeVis();
}

VValueNode::~VValueNode()
{
    delete contextMenu;
    delete widget;
}


void VValueNode::setShaderInput(bool tgl)
{
    ValueNode *node = static_cast<ValueNode*>(data);
    node->setShaderInput(tgl);
    if(tgl)
        setNodeColor(QColor(255, 135, 0));
    else
        setNodeColor(QColor(100, 100, 100));
}

void VValueNode::createContextMenu()
{
    ValueNode *node = static_cast<ValueNode*>(data);

    contextMenu = new QMenu;
    QAction *shaderinputAction =  contextMenu->addAction("Shader Parameter");
    shaderinputAction->setCheckable(true);
    shaderinputAction->setChecked(node->isShaderInput());
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
    proxy->setMinimumSize(1, 1);
    proxy->setContentsMargins(0, 0, 0, 0);
    //proxy->setPalette(QColor(0, 0, 0, 0));
    NodeWidth();
}

void VValueNode::NodeHeight(int numSockets)
{
    setNodeHeight(35);
};

void VValueNode::NodeWidth()    
{
    VNode::NodeWidth();
    if(getNodeWidth() < 80) setNodeWidth(80);
    if(proxy)
    {
        //proxy->resize(getNodeWidth(), 25);
        proxy->setPos(QPointF(4-getNodeWidth()/2, 2-SOCKET_HEIGHT));
        proxy->resize(getNodeWidth() - 15, 25);
    }
    if(widget)
        widget->resize(getNodeWidth() - 15, 25);
}

void VValueNode::updateNodeVis()    
{
    NodeWidth();
    NodeHeight(1);
    drawName();
    recalcNodeVis();
}

ColorButton::ColorButton(QColor init)
    : color(init)
{
    setPalette(QPalette(init));
    setFlat(true);
    connect(this, SIGNAL(clicked()), this, SLOT(setColor()));
}

void ColorButton::setColor()
{
    QColor newcolor = QColorDialog::getColor(color);
    if(!newcolor.isValid()) return;
    color = newcolor;
    setPalette(QPalette(newcolor));
    emit clicked(newcolor);
    update();
}

VColorValueNode::VColorValueNode(DNode *data)
    : VValueNode(data)
{
    ColorButton *cbutton = new ColorButton(static_cast<ColorValueNode*>(data)->getValue());
    cbutton->setMinimumSize(1, 1);
    cbutton->resize(20, 20);
    contextMenu->connect(cbutton, SIGNAL(clicked(QColor)), this, SLOT(setValue(QColor)));
    setValueEditor(cbutton);
    connect(cbutton, SIGNAL(clicked(QColor)), this, SLOT(emitNode()));
    connect(this, SIGNAL(valueChanged(DNode*)), FRG::Space, SIGNAL(linkChanged(DNode*)));
}

void VColorValueNode::emitNode()    
{
    emit valueChanged(data);
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
    connect(lineedit, SIGNAL(textChanged()), this, SLOT(emitNode()));
    connect(this, SIGNAL(valueChanged(DNode*)), FRG::Space, SIGNAL(linkChanged(DNode*)));

    lineedit->setText(static_cast<StringValueNode*>(data)->getValue());
}

void VStringValueNode::emitNode()    
{
   emit valueChanged(data); 
}

void VStringValueNode::setValue(QString string)
{
    StringValueNode* data = (StringValueNode*)this->data;
    data->setValue(string);
}

FloatValueSpinBox::FloatValueSpinBox()
{
    setRange(-1000, 1000);
    setDecimals(3);
    setSingleStep(0.1);
    setMinimumSize(1, 1);
    resize(50, 20);
}

VFloatValueNode::VFloatValueNode(DNode *data)
    :VValueNode(data), spinbox(new FloatValueSpinBox)
{
    contextMenu->connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    setValueEditor(spinbox);
    connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(emitNode()));
    connect(this, SIGNAL(valueChanged(DNode*)), FRG::Space, SIGNAL(linkChanged(DNode*)));

    spinbox->setValue(static_cast<FloatValueNode*>(data)->getValue());
}

void VFloatValueNode::emitNode()    
{
    emit valueChanged(data);
}

void VFloatValueNode::setValue(double fval)
{
    FloatValueNode *data = (FloatValueNode*)this->data;
    data->setValue(fval);
}

VectorValue::VectorValue()
{
    setContentsMargins(0, 0, 0, 0);
    lay = new QVBoxLayout();
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    spinx = new FloatValueSpinBox();
    spiny = new FloatValueSpinBox();
    spinz = new FloatValueSpinBox();
    lay->setMargin(0);
    lay->addWidget(spinx);
    lay->addWidget(spiny);
    lay->addWidget(spinz);
    setLayout(lay);
    connect(spinx, SIGNAL(valueChanged(double)), this, SLOT(emitValueChanged()));
    connect(spiny, SIGNAL(valueChanged(double)), this, SLOT(emitValueChanged()));
    connect(spinz, SIGNAL(valueChanged(double)), this, SLOT(emitValueChanged()));
}

void VectorValue::emitValueChanged()    
{
    Vector value(spinx->value(), spiny->value(), spinz->value());
    emit valueChanged(value); 
}

void VectorValue::setValue(Vector value)    
{
    spinx->setValue(value.x); 
    spiny->setValue(value.y); 
    spinz->setValue(value.z); 
}

VVectorValueNode::VVectorValueNode(DNode *data)
    :VValueNode(data), vec(new VectorValue())
{
    setValueEditor(vec);
    updateNodeVis();
    connect(vec, SIGNAL(valueChanged(Vector)), this, SLOT(setValue(Vector)));
    connect(vec, SIGNAL(valueChanged(Vector)), this, SLOT(emitNode()));
    connect(this, SIGNAL(valueChanged(DNode*)), FRG::Space, SIGNAL(linkChanged(DNode*)));
    vec->setValue(static_cast<VectorValueNode*>(data)->getValue());
}

void VVectorValueNode::emitNode()    
{
    emit valueChanged(data);
}

void VVectorValueNode::updateNodeVis()    
{
    NodeWidth();
    NodeHeight(1);
    drawName();
    recalcNodeVis();
}

void VVectorValueNode::NodeHeight(int numSockets)    
{
    VValueNode::NodeHeight(1); 
    setNodeHeight(getNodeHeight() + 40);
    if(widget)
        widget->resize(widget->size().width(), 60);
    if(proxy)
        proxy->setPos(proxy->pos().x(), proxy->pos().y() - 20);
}

void VVectorValueNode::setValue(Vector value)
{
    VectorValueNode *node = data->getDerived<VectorValueNode>();
    node->setValue(value);
}

VOutputNode::VOutputNode(DNode *data)
    :VNode(data)
{
    this->data = data;
    createMenu();

    changeDir();
}

void VOutputNode::createMenu()
{
    contextMenu = new QMenu;
    QAction *nameAction = contextMenu->addAction("set Shader Name");
    QAction *dirAction = contextMenu->addAction("set Shader Directory");
    QAction *writecodeAction = contextMenu->addAction("Create Code");
    QAction *compileAction = contextMenu->addAction("write and compile code");
    QAction *codeAction = contextMenu->addAction("view source code");

    connect(writecodeAction, SIGNAL(triggered()), this, SLOT(writeCode()));
    connect(nameAction, SIGNAL(triggered()), this, SLOT(changeName()));
    connect(dirAction, SIGNAL(triggered()), this, SLOT(changeDir()));
    connect(codeAction, SIGNAL(triggered()), this, SLOT(viewCode()));
}

void VOutputNode::changeName()
{
    OutputNode *data = (OutputNode*)this->data;
    bool ok;
    QString newname = QInputDialog::getText(FRG::Space->views().first(), "Shader Name", "Name", QLineEdit::Normal, "", &ok);
    if(ok)
        data->changeName(newname);
}

void VOutputNode::viewCode()    
{
    data->getDerived<OutputNode>()->getSourceEdit()->show();    
}

void VOutputNode::writeCode()
{
    OutputNode *data = static_cast<OutputNode*>(this->data);
    if(data->getFileName() == "")
        data->setFileName(QFileDialog::getExistingDirectory() + data->getShaderName() + ".sl");
    data->writeCode();
}

void VOutputNode::changeDir()
{
    OutputNode *data = static_cast<OutputNode*>(this->data);
    if(data->getShaderName() == "")changeName();
    data->setFileName(QFileDialog::getExistingDirectory() + data->getShaderName() + ".sl");
}

void VOutputNode::writeAndCompile()    
{
    OutputNode *data = static_cast<OutputNode*>(this->data);
    writeCode();
    data->compile();
}

void VOutputNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}
