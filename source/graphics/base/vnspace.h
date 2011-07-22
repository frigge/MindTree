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
    UndoRemoveNode(QList <DNode*> nodes, DNSpace *space);
    virtual void undo();
    virtual void redo();

private:
    QList<DNode*> nodes;
    QList<DNode*> redoNodes;
    DNSpace *space;
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
    void destroyContainer(QGraphicsItem *container);
    void moveIntoSpace(DNSpace *space);
    void createSpaceVis();
    void deleteSpaceVis();
	NodeLib *getNodeLib();
	NewNodeEditor *getNewNodeEditor();
	QPointF getMousePos();
	bool isLinkNodeMode();
	QList<DNode*> selectedNodes();
    bool isSelected(VNode *node);
    QPointF getSelectedItemsCenter();

public slots:
    void shownodelib();
    void createNode();
    void removeNode();
    void copy();
    void paste();
    void cut();
    void removeLink(VNodeLink *link);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void containerNode();

private:
    QMenu *NContextMenu;
    void ContextAddMenu();
    void updateLinks();
    ////QHash<DinSocket*, VNodeLink*> cachedLinks;
    QPointF mousePos;
    NodeLib *nodelib;
    NewNodeEditor *nodeedit;
    VNSocket *linksocket;
    VNodeLink *newlink;
    bool editNameMode;
	bool linkNodeMode;
    QList<DNode*> clipboard;
};

#endif // VNSPACE_H
