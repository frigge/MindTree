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
#include "source/data/base/frg.h"
#include "source/graphics/base/vnspace.h"

#include "QGraphicsSceneMouseEvent"
#include "QString"

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
    FRG::Space->addItem(this);
    text = new QGraphicsTextItem(data->getText());
    name = new QGraphicsTextItem(data->getName());
    name->setDefaultTextColor(QColor(255, 255, 255));
    name->setParentItem(this);
    text->setDefaultTextColor(QColor(0, 0, 0));
    text->setParentItem(this);
    name->setPos(0, -20);
    setPos(data->getPos());
    resizeHandle = new InfoBoxResizeHandle(this);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

VInfoBox::~VInfoBox()
{
}

QRectF VInfoBox::boundingRect() const
{
    return QRectF(0, 0, data->getWidth(), data->getHeight());
}

void VInfoBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)    
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(180, 180, 180, 180)));
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
    if(w > 50)
        data->setWidth(w);
    update();
}

void VInfoBox::setHeight(int h)    
{
    if(h > 50)
        data->setHeight(h); 
    update();
}
