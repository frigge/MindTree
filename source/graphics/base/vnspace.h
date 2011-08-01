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

#ifndef VNSPACE_H
#define VNSPACE_H

#include "QGraphicsScene"

#include "source/graphics/nodes/graphics_node.h"
#include "source/graphics/nodelink.h"
#include "source/graphics/nodelib.h"
#include "source/graphics/newnodeeditor.h"
#include "source/data/base/dnspace.h"
#include "source/data/undo/frg_generic_undo.h"

class UndoRemoveNode : public FRGUndoRedoObjectBase
{
public:
    UndoRemoveNode(QList<DNode*>nodes);
    ~UndoRemoveNode();
    virtual void undo();
    virtual void redo();

private:
    QList<DNode*> nodes;
};

class UndoDropNode : public FRGUndoRedoObjectBase
{
public:
    UndoDropNode(DNode *node);
    virtual void undo();
    virtual void redo();

private:
    DNode *node;
};

class UndoLink : public FRGUndoRedoObjectBase
{
public:
    UndoLink(DNodeLink dnlink, DNodeLink oldLink);
    virtual void undo();
    virtual void redo();

private:
    DNodeLink dnlink;
    DNodeLink oldLink;
};

class UndoRemoveLink : public FRGUndoRedoObjectBase
{
public:
    UndoRemoveLink(DNodeLink dnlink);
    virtual void undo();
    virtual void redo();

private:
    DNodeLink dnlink;
};

class UndoMoveNode : public FRGUndoRedoObjectBase
{
public:
    UndoMoveNode(NodeList nodes);
    virtual void undo();
    virtual void redo();

private:
    QHash<DNode*, QPointF> nodePositions;
};

class UndoBuildContainer : public FRGUndoRedoObjectBase
{
public:
    UndoBuildContainer(ContainerNode *contnode);
    virtual void undo();
    virtual void redo();

private:
    NodeList containerNodes;
    ContainerNode *contnode;
    QList<DNodeLink*> oldLinks;
};

class VNSpace : public QGraphicsScene
{
    Q_OBJECT
public:
    VNSpace();
    void addLink(VNSocket *final);
    void enterLinkNodeMode(VNSocket *socket);
    void leaveLinkNodeMode();
    void enterEditNameMode();
    void leaveEditNameMode();
    bool isEditNameMode();
    void moveIntoSpace(DNSpace *space);
    void createSpaceVis();
    void deleteSpaceVis();
    void cacheNodePositions(QList<DNode*>nodes);
	QPointF getMousePos();
	bool isLinkNodeMode();
	NodeList selectedNodes();
    bool isSelected(VNode *node);
    void removeNode(QList<DNode*>nodeList);
    void centerNodes(QList<DNode*>nodes);
    QPointF getCenter(QList<DNode*>nodes);
    void buildContainer();
    void unpackContainer();

public slots:
    void copy();
    void paste();
    void cut();
    void updateLinks();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void containerNode();

private:
    QMenu *NContextMenu;
    void ContextAddMenu();
    ////QHash<DinSocket*, VNodeLink*> cachedLinks;
    QPointF mousePos;
    NodeLib *nodelib;
    NewNodeEditor *nodeedit;
    VNSocket *linksocket;
    VNodeLink *newlink;
    bool editNameMode;
	bool linkNodeMode;
    QList<DNode*> nodeClipboard;
};

#endif // VNSPACE_H
