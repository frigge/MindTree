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
#include "source/data/base/dnspace.h"
#include "source/graphics/base/vnspace.h"

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
    DNode *pNode = (DNode*)parentItem();
    if((pNode->isContainer()))
    {
        ContainerNode *cNode = (ContainerNode*)pNode;
        if(cNode->ContainerData)
        {
            cNode->ContainerData->setName(toPlainText());
        }
    }
    if(scene())
    {
        DNSpace *space = (DNSpace*)scene();
        space->getSpaceVis()->leaveEditNameMode();
    }
    setTextInteractionFlags(Qt::NoTextInteraction);
    textCursor().clearSelection();
}

void NodeName::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(scene())
    {
        DNSpace *space = (DNSpace*)scene();
        space->getSpaceVis()->enterEditNameMode();
    }
    if (textInteractionFlags() == Qt::NoTextInteraction)
        setTextInteractionFlags(Qt::TextEditorInteraction);
    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

VNode::VNode(DNode *data)
{
    this->data = data;
    init();
}

void VNode::init()
{
    node_name = new NodeName("", this);
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
}

void VNode::drawName()
{
    node_name->setPlainText(data->nodeName);
    int node_width = NodeWidth();
    int node_height = NodeHeight(data->N_inSockets->size() + data->N_outSockets->size());
    node_name->setPos(0-(node_width/2)+4, -(node_height/2)-20);
}

int VNode::NodeWidth() const
{
    int node_width = 150;
    return node_width;
};

int VNode::NodeHeight(int numSockets) const
{
    int node_height = 0;
    if(!numSockets)
        node_height = 25;
    else
        node_height = 7 + (17*numSockets);
    return node_height;
};

QRectF VNode::boundingRect() const
{
    int node_height = NodeHeight(data->N_inSockets->size() + data->N_outSockets->size());
    node_height +=2;
    int node_width = NodeWidth();
    node_width += 2;
    return QRectF(0-(node_width/2), 0-(node_height/2), node_width, node_height);
};

void VNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    int node_width = NodeWidth();
    int node_height = NodeHeight(data->N_inSockets->size() + data->N_outSockets->size());
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
    painter->drawRect(-(node_width/2), 0-(node_height/2), node_width, node_height);

    int inSocket_pos;
    inSocket_pos = 0 - (node_height/2) + 2*space;

    foreach(DNSocket *insocket, *data->N_inSockets)
    {
        insocket->socketVis->setPos(2*socket_size-(node_width/2)+2*space, inSocket_pos-socket_size);
        painter->setPen(textColor);
        QRectF textrect(-node_width/2 + (socket_size+4*space), inSocket_pos, node_width-(socket_size+4*space), socket_size);
        painter->drawText(textrect, insocket->name, QTextOption(Qt::AlignLeft));
        inSocket_pos += socket_size + space;
    }

    int outSocket_pos;
    outSocket_pos = inSocket_pos + 2;

    foreach(DNSocket *out, *data->N_outSockets)
    {
        out->socketVis->setPos((node_width/2)+10, outSocket_pos-socket_size);
        painter->setPen(textColor);
        QRectF outtextrect(-node_width/2, outSocket_pos, node_width-(socket_size+4*space), socket_size);
        painter->drawText(outtextrect, out->name, QTextOption(Qt::AlignRight));
        outSocket_pos += socket_size + space;
    }
    drawName();
};

VContainerNode::VContainerNode(DNode *data)
    :VNode(data)
{
    if(data->NodeType == CONTAINER)
        createContextMenu();
}

void VContainerNode::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *addtolibaction = contextMenu->addAction("Add to Library");

    connect(addtolibaction, SIGNAL(triggered()), this, SLOT(addToLib()));
}

void VContainerNode::addToLib()
{
    ContainerNode* data = (ContainerNode*)this->data;
    data->addtolib();
}

void VContainerNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    ContainerNode *container = (ContainerNode*)data;
    if(!container->ContainerData)
        return;
    FRG::space->moveIntoSpace(container->ContainerData);
}

void VContainerNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(data->NodeType == CONTAINER)
        contextMenu->exec(event->screenPos());
}

VValueNode::VValueNode(DNode *data)
    :VNode(data)
{
}

void VValueNode::init()
{
    VNode::init();
    proxy = new QGraphicsProxyWidget;
    proxy->setParentItem(this);
    int width = NodeWidth();
    int height = NodeHeight(1);
    createContextMenu();
}

void VValueNode::createContextMenu()
{
    contextMenu = new QMenu;
    QAction *shaderinputAction = contextMenu->addAction("Shader Parameter");
    shaderinputAction->setCheckable(true);
    ValueNode *data = (ValueNode*)this->data;
    connect(shaderinputAction, SIGNAL(toggled(bool)), this, SLOT(setShaderInput(bool)));
}

void VValueNode::setShaderInput(bool isInput)
{
    ValueNode* data = (ValueNode*)this->data;
    data->setShaderInput(isInput);
}

void VValueNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    contextMenu->exec(event->screenPos());
}


void VValueNode::setValueEditor(QWidget *editor)
{
    widget = editor;
    proxy->setWidget(widget);
    widget->resize(NodeWidth() - 8, 25);
    proxy->setPos(QPointF(4-NodeWidth()/2, 8));
}

int VValueNode::NodeWidth() const
{
    int node_width = 150;
    return node_width;
};

int VValueNode::NodeHeight(int numSockets) const
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

VColorValueNode::VColorValueNode(DNode *data)
    : VValueNode(data)
{
}

void VColorValueNode::init()
{
    VValueNode::init();
    ColorButton *cbutton = new ColorButton;
    connect(cbutton, SIGNAL(clicked(QColor)), this, SLOT(setValue(QColor)));
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

}

void VStringValueNode::init()
{
    QLineEdit *lineedit = new QLineEdit;
    connect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(setValue(QString)));
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
}

void VFloatValueNode::init()
{
    QDoubleSpinBox *spinbox = new QDoubleSpinBox;
    spinbox->setRange(-1000, 1000);
    connect(spinbox, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
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

void VVectorValueNode::init()
{
}

void VVectorValueNode::setValue()
{

}

VOutputNode::VOutputNode(DNode *data)
    :VNode(data)
{
    this->data = data;
}

void VOutputNode::init()
{
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
