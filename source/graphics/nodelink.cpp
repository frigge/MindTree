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

#include "math.h"

#include "source/graphics/nodes/graphics_node_socket.h"
#include "source/graphics/nodes/graphics_node.h"
#include "source/data/base/frg.h"
#include "source/data/base/project.h"
#include "source/graphics/base/vnspace.h"

JointData::JointData(QPointF pos, DoutSocket *src, LinkJoint *vis)
    : idname(FRG::CurrentProject->registerItem(this, "link_helper")), source(src),
    parent(0), vis(vis), space(src->getNode()->getSpace())
{
    space->addJointData(this);
}

JointData::~JointData()
{
    space->removeJointData(this);
    FRG::CurrentProject->unregisterItem(this);
}

LinkJoint* JointData::getVis()
{
    if(!vis) return createVis();
    return vis;
}

void JointData::setVis(LinkJoint* joint)
{
    vis = joint;
}

LinkJoint* JointData::createVis()
{
    vis = new LinkJoint(this);
    return vis;
}

void JointData::setParent(JointData* p)    
{
    parent = p;
}

JointData* JointData::getParent()    const
{
    return parent;
}

void JointData::addInSocket(DinSocket *socket)    
{
    sockets.append(socket);
}

void JointData::removeInSocket(DinSocket *socket)    
{
    sockets.removeAll(socket);
}

QList<DinSocket*> JointData::getInSockets()
{
    return sockets;
}

DoutSocket* JointData::getSource()   const 
{
    return source;
}

void JointData::setPos(QPointF p)
{
    pos = p;
}

QPointF JointData::getPos()const
{
    return pos;
}


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

LinkJoint::LinkJoint(VNSocket *outSocket, QPointF center)
    : center(center), outSocket(outSocket), moving(false)
{
    FRG::Space->regJoint(this);
    setPos(center);
    setZValue(zValue() + 1);
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);

    data = new JointData(center, outSocket->getData()->toOut(), this);
}

LinkJoint::LinkJoint(JointData *data)
    : center(data->getPos()), outSocket(data->getSource()->getSocketVis()), moving(false), data(data)
{
    FRG::Space->regJoint(this);
    setPos(center);
    setZValue(zValue() + 1);
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);
}

LinkJoint::~LinkJoint()
{
    data->setVis(0);
    emit removed();
}

bool LinkJoint::isInBetween(QPointF j1, QPointF j2, QPointF pos)    
{
    return (((j1.x() < pos.x()) && (pos.x() < j2.x())) || ((j1.x() > pos.x()) && (pos.x() > j2.x()))) 
            && (((j1.y() < pos.y()) && (pos.y() < j2.y())) || ((j1.y() > pos.y()) && (pos.y() > j2.y())));
}

void LinkJoint::mousePressEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsItem::mousePressEvent(event);
    if(FRG::Space->isLinkNodeMode()){
        FRG::Space->addLink(this);
    }
    else if(event->modifiers() & Qt::CTRL) {
        FRG::Space->rmJoint(this);
        delete this;
    }
    else
        moving = true;
}

void LinkJoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsItem::mouseReleaseEvent(event);
    moving = false;
    data->setPos(pos());
}

void LinkJoint::mouseMoveEvent(QGraphicsSceneMouseEvent *event)    
{
    double thres = 10;
    QGraphicsItem::mouseMoveEvent(event);
    if(moving){
        foreach(VNodeLink *link, links){
            QPointF other;
            if(this == link->getStart()) {
                other = link->getEnd()->pos();
                if(link->getEnd()->type() == VNSocket::Type) other += ((VNSocket*)link->getEnd())->getData()->getNode()->getNodeVis()->pos();
            }
            else {
                other = link->getStart()->pos();
                if(link->getStart()->type() == VNSocket::Type) other += ((VNSocket*)link->getStart())->getData()->getNode()->getNodeVis()->pos();
            }
            double xpos = abs(pos().x() - other.x());
            double ypos = abs(pos().y() - other.y());
            if(xpos < thres) setPos(other.x(), pos().y());
            if(ypos < thres) setPos(pos().x(), other.y());
        }
    }
}

void LinkJoint::addLink(VNodeLink *link)    
{
    links.append(link);
    connect(link, SIGNAL(removeLink(VNodeLink*)), this, SLOT(rmLink(VNodeLink*)));
    if(this == link->getEnd() && link->getStart()->type() == LinkJoint::Type) data->setParent(((LinkJoint*)link->getStart())->getData());
    if(this == link->getStart() && link->getEnd()->type() == VNSocket::Type) data->addInSocket(((VNSocket*)link->getEnd())->getData()->toIn());
}

JointData* LinkJoint::getData()    
{
    return data;
}

void LinkJoint::rmLink(VNodeLink *link)    
{
    links.removeAll(link);
    disconnect(link, SIGNAL(removeLink(VNodeLink*)), this, SLOT(rmLink(VNodeLink*)));
    if(this == link->getEnd() && link->getStart()->type() == LinkJoint::Type) data->setParent(0);
    if(this == link->getStart() && link->getEnd()->type() == VNSocket::Type) data->removeInSocket(((VNSocket*)link->getEnd())->getData()->toIn());
}

QRectF LinkJoint::boundingRect() const
{
    return QRectF(-10, -10, 20, 20);
}

void LinkJoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)    
{
    painter->setRenderHint(QPainter::Antialiasing);
    QPointF diagVec = QPointF(-10, 20) - QPointF(20, -10);
    float length = sqrt(pow(diagVec.x(), 2) + pow(diagVec.y(), 2));
    QPointF ndiagVec(-diagVec.y() / length, diagVec.x() / length);

    QLinearGradient outlineCol(ndiagVec , -1 * ndiagVec);
    outlineCol.setColorAt(0, QColor(150, 150, 150));
    outlineCol.setColorAt(1, QColor(0, 0, 0));
    if(isSelected())
        painter->setPen(QColor(255, 145, 0));
    else
        painter->setPen(QPen(QBrush(outlineCol), 1));

    painter->setBrush(QBrush(QColor(50, 50, 50)));
    painter->drawEllipse(-5, -5, 10, 10);
}

VNSocket* LinkJoint::getOutSocket()    
{
    return outSocket;
}

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
    if(s->type() == LinkJoint::Type) {
        LinkJoint *joint = ((LinkJoint*)s);
        connect(joint, SIGNAL(removed()), this, SLOT(remove()));
        joint->addLink(this);
    }
    else{
        VNSocket *socket = ((VNSocket*)s);
        connect(socket->getData(), SIGNAL(removed()), this, SLOT(killVis()));
        connect(socket->getData()->toIn(), SIGNAL(disconnected()), this, SLOT(killVis()));
    }

    if(e->type() == LinkJoint::Type) {
        LinkJoint *joint = ((LinkJoint*)e);
        connect(joint, SIGNAL(removed()), this, SLOT(remove()));
        joint->addLink(this);
    }
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
    delete cMenu;
    disconnect();
}

void VNodeLink::initVNodeLink()
{
    setZValue(0);
    setFlag(ItemIsMovable, false);
    setFlag(ItemIsSelectable, false);
    //setFlag(ItemIgnoresTransformations, true);
    setAcceptHoverEvents(true);
    setAcceptsHoverEvents(true);
    //setGraphicsEffect(new QGraphicsDropShadowEffect);

    cMenu = new QMenu;
    QAction *removeAction = cMenu->addAction("Remove Link");
    connect(removeAction, SIGNAL(triggered()), this, SLOT(remove()));
    connect(removeAction, SIGNAL(triggered()), FRG::Space, SIGNAL(linkChanged()));
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

void VNodeLink::addJoint(QPointF pos)    
{
    VNodeLink *newin, *newout;
    LinkJoint *newjoint;
    newjoint = new LinkJoint(data->out->getSocketVis(), pos);
    newin = new VNodeLink(data);
    newout = new VNodeLink(data);
    //newin->setStart(start);
    //newin->setEnd(newjoint);
    newin->setRoute(start, newjoint);
    //newout->setStart(newjoint);
    //newout->setEnd(end);
    newout->setRoute(newjoint, end);
    delete this;
    //if(joints.isEmpty()) {
    //    joints.append(new LinkJoint(this, pos));
    //    return;
    //}

    //if(LinkJoint::isInBetween(out, joints.first()->pos(), pos)){
    //    joints.insert(0, new LinkJoint(this, pos));
    //    return;
    //}

    //int i=0;
    //foreach(LinkJoint *j, joints){
    //    if(i > joints.size() - 2) break;
    //    if(LinkJoint::isInBetween(j->pos(), joints.at(i+1)->pos(), pos)){
    //        joints.insert(i+1, new LinkJoint(this, pos));
    //        return;
    //    }
    //    i++;
    //}

    //if(LinkJoint::isInBetween(joints.last()->pos(), in, pos)){
    //    joints.append(new LinkJoint(this, pos));
    //    return;
    //}
}

void VNodeLink::mousePressEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsItem::mousePressEvent(event);
    if(event->modifiers() & Qt::CTRL) 
        addJoint(event->scenePos());
}

void VNodeLink::updateLink()
{
    if(start) out = start->pos();
    if(end)in = end->pos();

    if(start && start->type() == VNSocket::Type)
        out += ((VNSocket*)start)->getData()->getNode()->getNodeVis()->pos();
    if(end && end->type() == VNSocket::Type)
        in += ((VNSocket*)end)->getData()->getNode()->getNodeVis()->pos();
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

    //foreach(LinkJoint *j, joints){
    //    if(j->pos().x() < x) x = j->pos().x();
    //    if(j->pos().y() < y) y = j->pos().y();
    //    if(j->pos().x() > maxx) maxx = j->pos().x();
    //    if(j->pos().y() > maxy) maxy = j->pos().y();
    //}
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
    //foreach(LinkJoint *j, joints)
    //    path.lineTo(j->pos());
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
