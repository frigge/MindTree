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

#include "QGraphicsEffect"
#include "QGraphicsSceneContextMenuEvent"
#include "QDebug"
#include "math.h"
#include "cassert"

#include "data/data_nodelink.h"
#include "source/graphics/nodes/graphics_node_socket.h"
#include "vis_nodelink.h"

using namespace MindTree;

VNodeLink::VNodeLink()
    : start(0), end(0), data(0)
{
    initVNodeLink();
    c2 = QPointF((in.x()+out.x()/2), out.y());
    c1 = QPointF((in.x()+out.x()/2), in.y());
};

VNodeLink::VNodeLink(DNodeLink *data)
    : start(0), end(0), data(data)
{
    assert(data->in);
    assert(data->out);
    initVNodeLink();
}

DNodeLink VNodeLink::getData()
{
    return *data;
}

void VNodeLink::setLink(QPointF pos)    
{
    in = pos;
}

void VNodeLink::setData(DNodeLink value)
{
    data = new DNodeLink(value);
    assert(data->in);
    assert(data->out);
    if(!data->in->getArray())
        connect(data->in->toIn(), SIGNAL(linked(DoutSocket*)), this, SLOT(remove()));
    connect(this, SIGNAL(removeLink(VNodeLink *)), data->in->toIn(), SLOT(clearLink()));
}

void VNodeLink::setTemp(QGraphicsItem *tmp)    
{
    start = tmp;
}

void VNodeLink::setRoute(QGraphicsItem *s, QGraphicsItem *e)    
{
    start = s;
    end = e;
    VNSocket *socket = ((VNSocket*)s);
    connect(socket->getData(), SIGNAL(removed()), this, SLOT(killVis()));
    //connect(socket->getData()->toIn(), SIGNAL(disconnected()), this, SLOT(killVis()));
}

QGraphicsItem* VNodeLink::getStart()    const
{
    return start;
}

QGraphicsItem* VNodeLink::getEnd()    const
{
    return end;
}

VNodeLink::~VNodeLink()
{
    //FRG::Space->rmLink(this);
    emit removed(this);
    delete cMenu;
}

void VNodeLink::initVNodeLink()
{
    setZValue(zValue() + 1);
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    //setFlag(ItemIgnoresTransformations, true);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);
    //setGraphicsEffect(new QGraphicsDropShadowEffect);

    cMenu = new QMenu;
    QAction *removeAction = cMenu->addAction("Remove Link");
    connect(removeAction, SIGNAL(triggered()), this, SLOT(remove()));
    //connect(removeAction, SIGNAL(triggered()), FRG::Space, SIGNAL(linkChanged()));
    if(data)
    {
        assert(data->in);
        assert(data->out);
        if(!data->in->getArray()) {
            connect(data->in->toIn(), SIGNAL(linked(DoutSocket*)), this, SLOT(remove()));
            connect(this, SIGNAL(removeLink(VNodeLink *)), data->in->toIn(), SLOT(clearLink()));
        }
    }
}

void VNodeLink::remove()
{
    emit removeLink(this);
    delete this;
    //DSocket::removeLink(data->in, data->out);
}

void VNodeLink::killVis()    
{
    delete this;
}

void VNodeLink::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverEnterEvent(event);
    update();
}

void VNodeLink::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverLeaveEvent(event);
    update();
}

void VNodeLink::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    cMenu->exec(event->screenPos());
}

void VNodeLink::mousePressEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsItem::mousePressEvent(event);
}

void VNodeLink::updateLink()
{
    if(start) out = start->scenePos();
    if(end)in = end->scenePos();

    //c2.setX(in.x()+out.x()/2);
    //c2.setY(out.y());
    //c1.setX(in.x()+out.x()/2);
    //c1.setY(in.y());
};

QRectF VNodeLink::boundingRect() const
{
    qreal x, y, maxy, maxx;

    if (in.x() < out.x())
    { x = in.x();
        maxx = out.x();
    }
    else
    { x = out.x();
        maxx = in.x();
    };
    if (in.y() < out.y())
    { y = in.y();
        maxy = out.y();
    }
    else
    { y = out.y();
        maxy = in.y();
    };
    return QRectF(x, y, maxx - x, maxy - y);
};

QPointF VNodeLink::getmiddle(QPointF p1, QPointF p2)
{
    float midx = p1.x() > p2.x() ? p2.x() + (p1.x() - p2.x()) : p1.x() + (p2.x() - p1.x());
    float midy = p1.y() > p2.y() ? p2.y() + (p1.y() - p2.y()) : p1.y() + (p2.y() - p1.y());
    return QPointF(midx, midy);
}

QPainterPath VNodeLink::drawPath()const
{
    QPainterPath path;
    path.moveTo(in);
    if (in.y() == out.y())
    {
        path.lineTo(out);
        return path;
    }
    QPointF mids = QPointF(in.x() + ((out.x()-in.x())/2), in.y());
    QPointF mide = QPointF(in.x() + ((out.x()-in.x())/2), out.y());

    float dX, dY;
    dX = in.x() - out.x();
    dY = mids.y() < mide.y() ? mide.y() - mids.y() : mids.y() - mide.y();
    float saX, saY;
    saX = dX * .5;
    saY = dY * .5;
    QPointF mids_arc_start = QPointF(mids.x() + saX, mids.y());
    QPointF mids_arc_end = QPointF(mids.x(), mids.y() < mide.y() ? mids.y() + saY : mids.y() - saY);
    QPointF mide_arc_start = QPointF(mide.x(), mide.y() > mids.y() ? mide.y() - saY : mide.y() + saY);
    QPointF mide_arc_end = QPointF(mide.x() - saX, out.y());
    path.lineTo(mids_arc_start);
    path.cubicTo(mids, mids, mids_arc_end);
    path.lineTo(mide_arc_start);
    path.cubicTo(mide, mide, mide_arc_end);
    path.lineTo(out);

    return path;
}

QPainterPath VNodeLink::drawStraightPath()    const
{
    QPainterPath path(out);
    path.lineTo(in); 
    return path;
}

QPainterPath VNodeLink::shape()    const
{
    QPainterPath path = drawStraightPath();
    QPainterPathStroker stroke;
    stroke.setWidth(5);
    QPainterPath fpath = stroke.createStroke(path);
    return fpath;
}

void VNodeLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    updateLink();
    //painter->setRenderHint(QPainter::Antialiasing);
    //painter->setClipRect(option->exposedRect);
    QPen linkpen;
    //linkpen.setCosmetic(true);
    if (data)
    {
        linkpen.setWidthF(1.5);
        if(isUnderMouse())
            linkpen.setColor(QColor(255, 155, 155));
        else
            linkpen.setColor(QColor(150, 150, 150));
        linkpen.setStyle(Qt::SolidLine);
    }
    else
    {
        linkpen.setWidth(2);
        linkpen.setColor(QColor(255, 255, 255, 100));
        linkpen.setStyle(Qt::DotLine);
    }
    painter->setPen(linkpen);
    painter->setBrush(Qt::NoBrush);
    //painter->drawPath(drawPath());
    painter->drawPath(drawStraightPath());
};

