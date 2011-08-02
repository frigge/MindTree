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

#ifndef GRAPHICS_NODE_SOCKET_H
#define GRAPHICS_NODE_SOCKET_H

#include "QGraphicsItem"
#include "QObject"

#define SOCKET_WIDTH 20
#define SOCKET_HEIGHT 20

class DSocket;
class VNode;

class VNSocket : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    VNSocket(DSocket *, VNode*);
    ~VNSocket();
    enum {Type = UserType + 1};
    DSocket *data;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const {return Type;}
	void createNameVis();
    void killNameVis();
	int getWidth();
	void setWidth(int newwidth);
	void setHeight(int newHeight);
	int getHeight();
	int getSocketNameVisWidth();

public slots:
    void changeType();
    void changeName();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void createContextMenu();
    QMenu *contextMenu;

private:
	QGraphicsTextItem *socketNameVis;
	int width, height;
};


#endif // GRAPHCIS_NODE_SOCKET_H
