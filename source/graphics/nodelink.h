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

typedef struct DNodeLink
{
	DNodeLink(DSocket *in=0, DSocket *out=0, bool createvis=false);
    DNodeLink(const DNodeLink &link) : out(link.out), in(link.in), vis(link.vis){};
    ~DNodeLink();
    DoutSocket *out;
    DinSocket *in;
    VNodeLink *vis;
} DNodeLink;

class VNodeLink : public QGraphicsObject
{
    Q_OBJECT

public:
    VNodeLink(QPointF startP, QPointF endP);
    VNodeLink(DNodeLink *data);
    ~VNodeLink();
    DinSocket  *inSocket;
	DoutSocket *outSocket;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setlink(QPointF endP);
    void setlink(DinSocket *, DoutSocket *);
    void updateLink();

    static bool isCompatible(VNSocket *first, VNSocket *last);

public slots:
    void remove();

signals:
    void removeLink(VNodeLink *link);

private:
    DNodeLink *data;
    QPointF in, out, c1, c2;
    QPainterPath drawPath();
    QMenu *cMenu;
    void initVNodeLink();
    QPointF getmiddle(QPointF p1, QPointF p2);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

};

#endif // NODELINK_H
