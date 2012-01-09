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


#ifndef NODELINK_H
#define NODELINK_H

#include "QGraphicsObject"
#include "QGraphicsView"
#include "QMenu"

#include "source/data/nodes/data_node_socket.h"

class DinSocket;
class DoutSocket;
class DSocket;
class VNSocket;
class VNodeLink;
class LinkJoint;
class DNSpace;

class JointData 
{
public:
    JointData(QPointF pos, DoutSocket *src, LinkJoint *vis=0);
    ~JointData();

    LinkJoint* getVis();
    void setVis(LinkJoint* joint);
    LinkJoint* createVis();
    void setParent(JointData* p);
    JointData* getParent() const;
    DoutSocket* getSource() const;
    void addInSocket(DinSocket *socket);
    void removeInSocket(DinSocket *socket);
    QList<DinSocket*> getInSockets();
    void setPos(QPointF p);
    QPointF getPos()const;

private:
    mutable LinkJoint *vis;
    JointData *parent;
    DNSpace *space;
    QList<DinSocket*> sockets;
    DoutSocket *source;
    QPointF pos;
    QString idname;
};

typedef struct DNodeLink
{
	DNodeLink(DSocket *in=0, DSocket *out=0, bool createvis=false);
    DNodeLink(const DNodeLink &link) : out(link.out), in(link.in), vis(link.vis){};
    ~DNodeLink();
    DoutSocket *out;
    DinSocket *in;
    VNodeLink *vis;
} DNodeLink;

class LinkJoint : public QGraphicsObject
{
    Q_OBJECT
public:
    enum {Type = UserType + 3};
    int type() const {return Type;}
    LinkJoint(VNSocket *outSocket, QPointF center);
    LinkJoint(JointData *data);
    virtual ~LinkJoint();
    static bool isInBetween(QPointF j1, QPointF j2, QPointF pos);
    QRectF boundingRect() const;
    VNSocket* getOutSocket();
    void addLink(VNodeLink *link);
    JointData* getData();

public slots:
    void rmLink(VNodeLink *link);

signals:
    void removed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    JointData *data;
    VNSocket *outSocket;
    QList<VNodeLink *> links;
    QPointF center;
    bool moving;
};

class VNodeLink : public QGraphicsObject
{
    Q_OBJECT

public:
    VNodeLink();
    VNodeLink(DNodeLink *data);
    ~VNodeLink();
    DinSocket  *inSocket;
	DoutSocket *outSocket;
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void setData(DNodeLink data);
    DNodeLink getData();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void updateLink();
    //void setStart(QGraphicsItem *s);
    //void setEnd(QGraphicsItem *e);
    void setTemp(QGraphicsItem *tmp);
    void setRoute(QGraphicsItem *s, QGraphicsItem *e);
    QGraphicsItem* getEnd()const;
    QGraphicsItem* getStart()const;
    void addJoint(QPointF pos);
    void setLink(QPointF pos);

    static bool isCompatible(VNSocket *first, VNSocket *last);

public slots:
    void remove();
    void killVis();

signals:
    void removeLink(VNodeLink*);
    void removed(VNodeLink*);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    QPainterPath drawPath()const;
    QPainterPath drawStraightPath()const;
    void initVNodeLink();
    QPointF getmiddle(QPointF p1, QPointF p2);

    DNodeLink *data;
    QGraphicsItem *start, *end;
    QPointF in, out, c1, c2;
    QMenu *cMenu;
};

#endif // NODELINK_H
