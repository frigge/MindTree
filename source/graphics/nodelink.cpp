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

#include "nodelink.h"

#include "QGraphicsEffect"
#include "QMenu"
#include "QGraphicsSceneContextMenuEvent"
#include "QDebug"

#include "source/graphics/nodes/graphics_node_socket.h"
#include "source/graphics/nodes/graphics_node.h"
#include "source/data/base/frg.h"
#include "source/graphics/base/vnspace.h"

DNodeLink::DNodeLink(DSocket *in, DSocket *out, bool createvis)
    : out(out->toOut()), in(in->toIn()), vis(0)
{
    if(createvis)
    {
        vis = new VNodeLink(this);
        FRG::Space->addItem(vis);
    }
}

DNodeLink::~DNodeLink()
{
    if(vis)
    {
        FRG::Space->removeItem(vis);
        delete vis;
    }
}

VNodeLink::VNodeLink(QPointF startP, QPointF endP)
{
    initVNodeLink();
    in = startP;
    out = endP;
    c2 = QPointF((in.x()+out.x()/2), out.y());
    c1 = QPointF((in.x()+out.x()/2), in.y());
};

VNodeLink::VNodeLink(DNodeLink *data)
    : data(data)
{
    initVNodeLink();
    setlink(data->in, data->out);
}

VNodeLink::~VNodeLink()
{
    delete cMenu;
    disconnect();
}

void VNodeLink::initVNodeLink()
{
    inSocket = 0;
    outSocket = 0;
    setZValue(0);
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    //setFlag(ItemIgnoresTransformations, true);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);
    //setGraphicsEffect(new QGraphicsDropShadowEffect);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);

    cMenu = new QMenu;
    QAction *removeAction = cMenu->addAction("Remove Link");
    connect(removeAction, SIGNAL(triggered()), this, SLOT(remove()));
    connect(removeAction, SIGNAL(triggered()), FRG::Space, SIGNAL(linkChanged()));
}

void VNodeLink::remove()
{
    DNodeLink dnlink(data->in, data->out);
    if(dnlink.in->getVariable())
        dnlink.in = dnlink.in->getNode()->getVarSocket()->toIn();
    if(dnlink.out->getVariable())
        dnlink.out = dnlink.out->getNode()->getVarSocket()->toOut();
    FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoRemoveLink(dnlink));
    //emit removeLink(this);
    DSocket::removeLink(data->in, data->out);
}

void VNodeLink::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    update();
}

void VNodeLink::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    update();
}

void VNodeLink::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    cMenu->exec(event->screenPos());
}

void VNodeLink::setlink(QPointF endP)
{
    out = endP;
    c2 = QPointF((in.x()+out.x()/2), out.y());
    c1 = QPointF((in.x()+out.x()/2), in.y());
};

void VNodeLink::setlink(DinSocket *in, DoutSocket *out)
{
	inSocket = in;
	outSocket = out;
    updateLink();
};

void VNodeLink::updateLink()
{
    if(!inSocket ||!outSocket)
        return;
    if(!inSocket->getNode() ||!outSocket->getNode())
        return;
    const VNode *pin, *pout;
    pin = inSocket->getNode()->getNodeVis();
    pout = outSocket->getNode()->getNodeVis();
    if(!pin
       ||!pout)
    {
        return;
    }
    in = inSocket->getSocketVis()->pos() + pin->pos();
    in.setX(in.x());
    in.setY(in.y());
    const VNSocket *socketVis = outSocket->getSocketVis();
    out = socketVis->pos();
    out += pout->pos();
    out.setX(out.x());
    out.setY(out.y());
    c2.setX(in.x()+out.x()/2);
    c2.setY(out.y());
    c1.setX(in.x()+out.x()/2);
    c1.setY(in.y());
};

QRectF VNodeLink::boundingRect() const
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

QPointF VNodeLink::getmiddle(QPointF p1, QPointF p2)
{
    float midx = p1.x() > p2.x() ? p2.x() + (p1.x() - p2.x()) : p1.x() + (p2.x() - p1.x());
    float midy = p1.y() > p2.y() ? p2.y() + (p1.y() - p2.y()) : p1.y() + (p2.y() - p1.y());
    return QPointF(midx, midy);
}

QPainterPath VNodeLink::drawPath()
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

void VNodeLink::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    //painter->setClipRect(option->exposedRect);
    QPen linkpen;
    //linkpen.setCosmetic(true);
    if (outSocket)
    {
        linkpen.setWidthF(1.5);
        if(isUnderMouse())
            linkpen.setColor(QColor(255, 155, 155));
        else
            linkpen.setColor(QColor(200, 200, 200));
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
    painter->drawLine(out, in);
};
