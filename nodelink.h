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

#include "QGraphicsItem"
#include "QGraphicsView"
#include "QMenu"
#include "QObject"

#include "node.h"

class NodeLink : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    NodeLink(QPointF startP, QPointF endP);
    NodeLink();
    ~NodeLink();
    NSocket *inSocket, *outSocket;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setlink(QPointF endP);
    void setlink(NSocket *in, NSocket *out);
    void updateLink();

    static bool isCompatible(NSocket *first, NSocket *last);

public slots:
    void remove();

signals:
    void removeLink(NodeLink *link);

private:
    QPointF in, out, c1, c2;
    QPainterPath drawPath();
    QMenu *cMenu;
    void initNodeLink();
    QPointF getmiddle(QPointF p1, QPointF p2);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

};

QDataStream & operator<<(QDataStream &stream, NodeLink *nlink);
QDataStream & operator>>(QDataStream &stream, NodeLink **nlink);

#endif // NODELINK_H
