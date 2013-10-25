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

#ifndef VIS_NODELINK_3QVUV9ZO

#define VIS_NODELINK_3QVUV9ZO
#include "QGraphicsObject"
#include "QGraphicsView"
#include "QMenu"

namespace MindTree
{
class DNodeLink;
class DinSocket;
class DoutSocket;
class VNSocket;
} /* MindTree */


class VNodeLink : public QGraphicsObject
{
    Q_OBJECT

public:
    VNodeLink();
    VNodeLink(MindTree::DNodeLink *data);
    ~VNodeLink();
    MindTree::DinSocket  *inSocket;
    MindTree::DoutSocket *outSocket;
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void setData(MindTree::DNodeLink data);
    MindTree::DNodeLink getData();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void updateLink();
    //void setStart(QGraphicsItem *s);
    //void setEnd(QGraphicsItem *e);
    void setTemp(QGraphicsItem *tmp);
    void setRoute(QGraphicsItem *s, QGraphicsItem *e);
    QGraphicsItem* getEnd()const;
    QGraphicsItem* getStart()const;
    void setLink(QPointF pos);

    static bool isCompatible(MindTree::VNSocket *first, MindTree::VNSocket *last);

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

    MindTree::DNodeLink *data;
    QGraphicsItem *start, *end;
    QPointF in, out, c1, c2;
    QMenu *cMenu;
};


#endif /* end of include guard: VIS_NODELINK_3QVUV9ZO */
