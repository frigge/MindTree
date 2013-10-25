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

#include "vis_info_box.h"
#include "source/data/data_info_box.h"
#include "data/frg.h"
#include "source/graphics/base/vnspace.h"

#include "QGraphicsSceneMouseEvent"
#include "QString"

InfoText::InfoText(QString text, VInfoBox *vb)
    : QGraphicsTextItem(text), vb(vb)
{
}

InfoText::~InfoText()
{
}

void InfoText::focusOutEvent(QFocusEvent *event)
{
    QGraphicsTextItem::focusOutEvent(event);
    setTextInteractionFlags(Qt::NoTextInteraction); 
    vb->setInfoText(toPlainText());
}

void InfoText::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsTextItem::mouseDoubleClickEvent(event);
    setTextInteractionFlags(Qt::TextEditorInteraction);
}

InfoBoxResizeHandle::InfoBoxResizeHandle(VInfoBox *box)
    : box(box), dragMode(false)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    setPos(box->getWidth(), box->getHeight());

    setParentItem(box);
}

InfoBoxResizeHandle::~InfoBoxResizeHandle()
{
}

QRectF InfoBoxResizeHandle::boundingRect() const
{
    return QRectF(-30, -30, 30, 30);
}

void InfoBoxResizeHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)    
{
    painter->setPen(QPen(QColor(50, 50, 50, 200)));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());
}

void InfoBoxResizeHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)    
{
    dragMode = true;
    tmpPos = event->scenePos();
    QGraphicsItem::mousePressEvent(event);
}

void InfoBoxResizeHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)    
{
    if(dragMode){
        QPointF delta = event->scenePos() - tmpPos;
        int w, h;
        w = box->getWidth();
        h = box->getHeight();
        box->setWidth(w + delta.x());
        box->setHeight(h + delta.y());
        if(w != box->getWidth())
            setX(box->getWidth());
        if(h != box->getHeight())
            setY(box->getHeight());
        tmpPos = event->scenePos();
    }
    else
        QGraphicsItem::mouseMoveEvent(event);
}

void InfoBoxResizeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)    
{
    dragMode = false;
    tmpPos = QPointF();
    QGraphicsItem::mouseReleaseEvent(event);
}

VInfoBox::VInfoBox(DInfoBox *data)
    : data(data)
{
    //FRG::Space->addItem(this);
    text = new InfoText(data->getText(), this);
    text->setDefaultTextColor(QColor(255, 255, 255));
    text->setParentItem(this);
    setPos(data->getPos());
    resizeHandle = new InfoBoxResizeHandle(this);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    setZValue(0);

    createMenu();
}

VInfoBox::~VInfoBox()
{
    data->setVis(0);
}

void VInfoBox::createMenu()    
{
    menu = new QMenu();
    QAction *editTextAction = menu->addAction("Edit Note");
    connect(editTextAction, SIGNAL(triggered()), this, SLOT(editText()));
}

void VInfoBox::editText()
{
    text->setTextInteractionFlags(Qt::TextEditorInteraction);
    text->setFocus();
}

void VInfoBox::setInfoText(QString text)    
{
    data->setText(text);
}

QRectF VInfoBox::boundingRect() const
{
    return QRectF(0, 0, data->getWidth(), data->getHeight());
}

void VInfoBox::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    menu->exec(event->screenPos());
}

void VInfoBox::mousePressEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsItem::mousePressEvent(event);
    tmpPos = event->scenePos();
    movingItems = collidingItems();
    //foreach(QGraphicsItem *item, FRG::Space->selectedItems())
    //    item->setSelected(false);
}

void VInfoBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsItem::mouseMoveEvent(event);
    QPointF delta = event->scenePos() - tmpPos;
    foreach(QGraphicsItem *item, movingItems)
        if(item->type() == VNode::Type
            ||item->type() == VInfoBox::Type)
            item->setPos(item->pos() + delta);
    tmpPos = event->scenePos();
}

void VInfoBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsItem::mouseReleaseEvent(event); 
    data->setPos(pos());
}

void VInfoBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)    
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(180, 180, 180, 50)));
    painter->drawRect(boundingRect());
}

DInfoBox* VInfoBox::getData()
{
    return data;
}

void VInfoBox::setData(DInfoBox* value)
{
    data = value;
}

int VInfoBox::getWidth()   const 
{
    return data->getWidth();
}

int VInfoBox::getHeight()  const  
{
    return data->getHeight();
}

void VInfoBox::setWidth(int w)    
{
    if(w > 50) data->setWidth(w);
    update();
}

void VInfoBox::setHeight(int h)    
{
    if(h > 50) data->setHeight(h); 
    update();
}
