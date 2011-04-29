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
#include "QMenu"
#include "QGraphicsSceneContextMenuEvent"

#include "nodelink.h"
#include "node.h"
#include "shader_space.h"

NodeLink::NodeLink(QPointF startP, QPointF endP)
{
    initNodeLink();
    in = startP;
    out = endP;
    c2 = QPointF((in.x()+out.x()/2), out.y());
    c1 = QPointF((in.x()+out.x()/2), in.y());
};

NodeLink::NodeLink()
{
    initNodeLink();
}

NodeLink::~NodeLink()
{
    delete cMenu;
    disconnect();
}

void NodeLink::initNodeLink()
{
    inSocket = 0;
    outSocket = 0;
    setZValue(zValue()-0.1);
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);
    //setGraphicsEffect(new QGraphicsDropShadowEffect);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);

    cMenu = new QMenu;
    QAction *removeAction = cMenu->addAction("Remove Link");
    connect(removeAction, SIGNAL(triggered()), this, SLOT(remove()));
}

QDataStream &operator <<(QDataStream &stream, NodeLink *nlink)
{
    stream<<(qint16)nlink->inSocket->ID<<(qint16)nlink->outSocket->ID;
    return stream;
}

QDataStream &operator >>(QDataStream &stream, NodeLink **nlink)
{
    qint16 inID, outID;
    stream>>inID>>outID;
    NodeLink *newlink = new NodeLink();
    NSocket *insocket = NSocket::loadIDMapper[inID];
    NSocket *outsocket = NSocket::loadIDMapper[outID];
    newlink->inSocket = insocket;
    newlink->outSocket = outsocket;
    *nlink = newlink;
    return stream;
}

void NodeLink::remove()
{
    emit removeLink(this);
}

void NodeLink::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    update();
}

void NodeLink::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    update();
}

void NodeLink::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    cMenu->exec(event->screenPos());
}

void NodeLink::setlink(QPointF endP)
{
    out = endP;
    c2 = QPointF((in.x()+out.x()/2), out.y());
    c1 = QPointF((in.x()+out.x()/2), in.y());
};

void NodeLink::setlink(NSocket *first, NSocket *last)
{
    if (first->Socket.dir == IN)
    {
        inSocket = first;
        outSocket = last;
    }
    else
    {
        inSocket = last;
        outSocket = first;
    }
    inSocket->addLink(this);
    outSocket->addLink(this);
    updateLink();
};

bool NodeLink::isCompatible(NSocket *first, NSocket *last)
{
    NSocket * in, *out;
    if (!first || !last)
        return false;
    if (first->Socket.dir == IN)
    {
        in = first;
        out = last;
    }
    else
    {
        in = last;
        out = first;
    }
    if(in->Socket.cntdSockets.size() > 0)
        return false;
    if(in->Socket.type == VARIABLE || in->Socket.type == out->Socket.type || out->Socket.type == VARIABLE)
        return true;
    else if((in->Socket.type == NORMAL && out->Socket.type == VECTOR) || (in->Socket.type == VECTOR && out->Socket.type == NORMAL))
        return true;
    else
        return false;
}

void NodeLink::updateLink()
{
    if(!inSocket || !outSocket)
    {
        remove();
        return;
    }
    QGraphicsItem *pin, *pout;
    pin = inSocket->Socket.node;
    pout = outSocket->Socket.node;
    in = inSocket->pos() + pin->pos();
    in.setX(in.x()-15);
    in.setY(in.y()+22);
    out = outSocket->pos() + pout->pos();
    out.setX(out.x()-15);
    out.setY(out.y()+22);
    c2.setX(in.x()+out.x()/2);
    c2.setY(out.y());
    c1.setX(in.x()+out.x()/2);
    c1.setY(in.y());
};

QRectF NodeLink::boundingRect() const
{
    qreal x, y, width, height;
    if (in.x() < out.x())
    {
        x = in.x();
        width = out.x() - in.x();
    }
    else
    {
        x = out.x();
        width = in.x() - out.x();
    };
    if (in.y() < out.y())
    {
        y = in.y();
        height = out.y() - in.y();
    }
    else
    {
        y = out.y();
        height = in.y() - out.y();
    };
    return QRectF(x, y, width, height);
};

QPointF NodeLink::getmiddle(QPointF p1, QPointF p2)
{
    float midx = p1.x() > p2.x() ? p2.x() + (p1.x() - p2.x()) : p1.x() + (p2.x() - p1.x());
    float midy = p1.y() > p2.y() ? p2.y() + (p1.y() - p2.y()) : p1.y() + (p2.y() - p1.y());
    return QPointF(midx, midy);
}

QPainterPath NodeLink::drawPath()
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

void NodeLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::white);
    QPen linkpen;
    if (outSocket)
    {
        linkpen.setWidth(1);
        if(isUnderMouse())
            linkpen.setColor(QColor(255, 255, 255));
        else
            linkpen.setColor(QColor(255, 255, 255, 150));
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
    painter->drawPath(drawPath());
};
