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

#include "source/data/nodes/data_node_socket.h"

#define SOCKET_WIDTH 13
#define SOCKET_HEIGHT 13

class DSocket;
class VNode;
class VNodeUpdateCallback;
class VNSocket;

class SocketTypeAction : public QAction
{
    Q_OBJECT
public:
    SocketTypeAction(DSocket *socket, socket_type t, QObject *parent);

public slots:
    void setType();

private:
    DSocket *socket;
    socket_type type;
};

class SocketUnlinkButton;
class SocketChangeTypeButton;
class SocketButtonContainer : public QGraphicsItem
{
public:
    SocketButtonContainer(VNSocket *socket);
    virtual ~SocketButtonContainer();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);
    QRectF boundingRect() const;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    VNSocket *socket;
    SocketUnlinkButton *unlinkButton;
    SocketChangeTypeButton *changeTypeButton;
};

class SocketChangeTypeButton : public QGraphicsItem
{
public:
    SocketChangeTypeButton (VNSocket *socket);
    ~SocketChangeTypeButton ();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    QRectF boundingRect() const;

private:
    void createMenu();
    QList<QAction*> createActions();
    VNSocket *socket;
    QMenu *typeMenu;
};

class SocketUnlinkButton : public QGraphicsItem
{
public:
    SocketUnlinkButton (VNSocket *socket);
    ~SocketUnlinkButton();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    QRectF boundingRect() const;

private:
    VNSocket *socket;
};

class VNSocket : public QGraphicsObject
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
    QColor getColor();

public slots:
    void changeType();
    void changeName();
    void setArray();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void createContextMenu();
    void createArrCMEntry();
    void removeArrCMEntry();
    QMenu *contextMenu;

private:
    SocketButtonContainer *socketButtons;
    QColor color;
	QGraphicsTextItem *socketNameVis;
    DSocket *data;
    VNodeUpdateCallback *cb;
    QAction *expandAction;
	int width, height;
    bool drawName;
    bool blockContextMenu;
    bool visible;
    QPointF dragstartpos;
};

#endif // GRAPHCIS_NODE_SOCKET_H
