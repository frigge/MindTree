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

#ifndef GRAPHICS_NODE_H
#define GRAPHICS_NODE_H

#include "QHash"
#include "QPushButton"
#include "QCheckBox"
#include "QGridLayout"
#include "QGraphicsTextItem"
#include "QGraphicsView"
#include "QGraphicsItem"
#include "QObject"
#include "QAction"
#include "QDoubleSpinBox"

#include "source/graphics/nodes/graphics_node_socket.h"

using namespace MindTree;
namespace MindTree {
class ContainerNode;
class DSocket;
}

class ChooseInputAction : public QAction
{
    Q_OBJECT
public:
    ChooseInputAction(MindTree::DSocket *socket, VNode *node);

public slots:
    void addLink();

private:
    VNode *vnode;
    DSocket *socket;
};

class NodeName : public QGraphicsTextItem
{
public:
    NodeName(QString name, QGraphicsItem *parent);

protected:
    void focusOutEvent(QFocusEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
};

class AbstractNodeGraphics : public QGraphicsObject
{
public:
    AbstractNodeGraphics(MindTree::DNode_ptr data);
    virtual ~AbstractNodeGraphics();

private:
    MindTree::DNode_ptr data;
};

class PyNodeGraphics : public AbstractNodeGraphics
{
public:
    PyNodeGraphics(MindTree::DNode* data);
    virtual ~PyNodeGraphics();

protected:
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
};

class VNode : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QColor nodeColor READ getNodeColor WRITE setNodeColor)
public:
    VNode(MindTree::DNode *data);
    virtual ~VNode();
    MindTree::DNode *data;
    VNSocket* getSocketVis(DSocket *socket);
    enum {Type = UserType + 2};
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * = 0);
    int type() const {return Type;}
    NodeName *node_name;
    void drawName();
	void cacheSocketSize();
	int getNodeWidth()const;
	void setNodeWidth(int w);
	void setNodeHeight(int h);
	int getNodeHeight()const;
    void setNodeColor(QColor col);
    QColor getNodeColor();

public slots:
    virtual void updateNodeVis();

protected:
	virtual void recalcNodeVis();
    virtual void NodeWidth();
    virtual void NodeHeight(int numSockets);
    void createInputMenu(socket_type t, QPoint pos);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QMenu *inpM;
	unsigned short socket_size;
	unsigned short node_width, node_height;
    //VNodeUpdateCallback *cb;
    bool moving;
    QColor nodeColor;
};

class VContainerNode : public VNode
{
    Q_OBJECT
public:
    VContainerNode(MindTree::DNode*);
    ~VContainerNode();

public slots:
    void addToLib();

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    void createContextMenu();
    QMenu *contextMenu;
};
#endif // GRAPHICS_NODE_H
