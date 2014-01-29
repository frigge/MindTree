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
#include "source/graphics/base/vis_nodelink.h"
#include "data/data_nodelink.h"
#include "source/graphics/nodelib.h"
#include "source/graphics/newnodeeditor.h"
#include "data/dnspace.h"
#include "source/data/undo/frg_generic_undo.h"
#include "source/data/data_info_box.h"

class UndoRemoveNode : public FRGUndoRedoObjectBase
{
public:
    UndoRemoveNode(QList<MindTree::DNode*>nodes);
    ~UndoRemoveNode();
    virtual void undo();
    virtual void redo();

private:
    QList<MindTree::DNode*> nodes;
};

class UndoDropNode : public FRGUndoRedoObjectBase
{
public:
    UndoDropNode(MindTree::DNode *node);
    virtual void undo();
    virtual void redo();

private:
    MindTree::DNode *node;
};

class UndoLink : public FRGUndoRedoObjectBase
{
public:
    UndoLink(MindTree::DNodeLink dnlink, MindTree::DNodeLink oldLink);
    virtual void undo();
    virtual void redo();

private:
    MindTree::DNodeLink dnlink;
    MindTree::DNodeLink oldLink;
};

class UndoRemoveLink : public FRGUndoRedoObjectBase
{
public:
    UndoRemoveLink(MindTree::DNodeLink dnlink);
    virtual void undo();
    virtual void redo();

private:
    MindTree::DNodeLink dnlink;
};

class UndoMoveNode : public FRGUndoRedoObjectBase
{
public:
    UndoMoveNode(NodeList nodes);
    virtual void undo();
    virtual void redo();

private:
    QHash<MindTree::DNode*, QPointF> nodePositions;
};

class UndoBuildContainer : public FRGUndoRedoObjectBase
{
public:
    UndoBuildContainer(MindTree::ContainerNode *contnode);
    virtual void undo();
    virtual void redo();

private:
    MindTree::NodeList containerNodes;
    MindTree::ContainerNode *contnode;
    QList<MindTree::DNodeLink> oldLinks;
};

typedef struct ContainerCache
{
    ContainerCache(MindTree::ContainerNode *contnode);
    MindTree::ContainerNode *contnode;
    NodeList containerNodes;
    QList<MindTree::DNodeLink> oldLinks;
} ContainerCache;

class UndoUnpackContainer : public FRGUndoRedoObjectBase
{
public:
    UndoUnpackContainer(QList<MindTree::ContainerNode*>contnodes);
    virtual void undo();
    virtual void redo();

private:
    QList<ContainerCache> containers;
};

class VNSpace : public QGraphicsScene
{
    Q_OBJECT
public:
    VNSpace();
    void expandTop(float val);
    void shrinkTop(float val);
    void expandBottom(float val);
    void shrinkBottom(float val);
    void expandLeft(float val);
    void shrinkLeft(float val);
    void expandRight(float val);
    void shrinkRight(float val);

    void addLink(VNSocket *final);
    void enterLinkNodeMode(VNSocket *socket);
    void leaveLinkNodeMode();
    void cancelLinkNodeMode();
    VNSocket* getLinkSocket();
    void enterEditNameMode();
    void leaveEditNameMode();
    bool isEditNameMode();
    void moveIntoSpace(DNSpace *space);
    void createSpaceVis();
    void createNodeVis(DNode *node);
    void updateNodeVis(DNSpace *space);
    void cacheNodePositions(QList<MindTree::DNode*>nodes);
	QPointF getMousePos();
	bool isLinkNodeMode();
	NodeList selectedNodes();
    bool isSelected(VNode *node);
    void removeNode(QList<MindTree::DNode*>nodeList);
    void centerNodes(QList<MindTree::DNode*>nodes);
    QPointF getCenter(QList<MindTree::DNode*>nodes);
    void buildContainer();
    void unpackContainer();
    void regLink(VNodeLink *l);
    void rmLink(VNodeLink *l);

signals:
    void linkChanged();
    void linkChanged(MindTree::DNode*);
    void nodeDroped(MindTree::DNode*);

public slots:
    void createVis(MindTree::DNode* node);
    void deleteNodeVis(MindTree::DNode* node);
    void copy();
    void paste();
    void cut();
    void updateLinks();
    void moveIntoRootSpace();

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void containerNode();
    void deleteLinkVis();
    void deleteInfoBoxes();
    void createLinkVis();
    void createLinkVis(DNodeLink link);
    void createInfoBoxes();
    void deleteSpaceVis();

private:
    void connectCallbacks();
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
    QList<MindTree::DNode*> nodeClipboard;
    QList<VNodeLink*> links;
    QHash<MindTree::DNode*, QGraphicsItem*> visibleNodes;
    DNSpace *spaceData;
};

#endif // VNSPACE_H
