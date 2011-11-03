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
#include "QAction"

#define SOCKET_WIDTH 13
#define SOCKET_HEIGHT 13

class DSocket;
class VNode;
class VNodeUpdateCallback;

class VNSocket : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    VNSocket(DSocket *, VNode*);
    ~VNSocket();
    enum {Type = UserType + 1};
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const {return Type;}
	void createNameVis();
    void killNameVis();
	int getWidth() const;
	void setWidth(int newwidth);
	void setHeight(int newHeight);
	int getHeight() const;
	int getSocketNameVisWidth() const;
    DSocket *getData() const;
    void updateNameVis();
    void setDrawName(bool draw);
    void setBlockContextMenu(bool block);
    void setVisible(bool vis);

public slots:
    void changeType();
    void changeName();
    void setArray();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void createContextMenu();
    void createArrCMEntry();
    void removeArrCMEntry();
    QMenu *contextMenu;

private:
	QGraphicsTextItem *socketNameVis;
    DSocket *data;
    VNodeUpdateCallback *cb;
    QAction *expandAction;
	int width, height;
    bool drawName;
    bool blockContextMenu;
    bool visible;
};

#endif // GRAPHCIS_NODE_SOCKET_H
