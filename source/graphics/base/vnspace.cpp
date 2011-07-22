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

#include "vnspace.h"

#include "QGraphicsSceneMouseEvent"
#include "QKeyEvent"

#include "source/data/base/frg.h"
#include "source/data/base/frg_shader_author.h"
#include "source/data/base/project.h"
#include "source/data/nodes/buildin_nodes.h"

UndoRemoveNode::UndoRemoveNode(QList<DNode*>nodes, DNSpace *space)
    : space(space)
{
    foreach(DNode *node, nodes)
    {
        FRG::CurrentProject->setNodePosition(node, node->getNodeVis()->pos());
        this->nodes.append(DNode::copy(node));
    }
}

void UndoRemoveNode::undo()
{
    foreach(DNode *node, nodes)
    {
        DNode *restoreNode = DNode::copy(node);
        redoNodes.append(restoreNode);
        space->addNode(restoreNode);
        if(FRG::SpaceDataInFocus == space)
            FRG::Space->addItem(restoreNode->createNodeVis());
    }
}

void UndoRemoveNode::redo()    
{
    foreach(DNode *node, redoNodes)
        space->removeNode(node);
    redoNodes.clear();
}

VNSpace::VNSpace()
    : newlink(0), nodelib(0), nodeedit(0), editNameMode(false), linkNodeMode(false)
{
    qreal space_width, space_height;
    if (itemsBoundingRect().width() < 1000)
        space_width = 1000;
    else
        space_width = itemsBoundingRect().width();
    if (itemsBoundingRect().height() < 1000)
        space_height = 1000;
    else
        space_height = itemsBoundingRect().height();
    QRectF space_rect = QRectF(0-(space_width/2), 0-(space_height/2), space_width, space_height);
    setSceneRect(space_rect);
}

void VNSpace::ContextAddMenu()
{
    NContextMenu = new QMenu;

    QAction *newNodeAction = NContextMenu->addAction("New Node");
    connect(newNodeAction, SIGNAL(triggered()), this, SLOT(createNode()));
};

void VNSpace::removeNode()
{
    FRG::CurrentProject->registerUndoRedoObject(new UndoRemoveNode(selectedNodes(), FRG::SpaceDataInFocus));
    foreach(DNode *node, selectedNodes())
        FRG::SpaceDataInFocus->removeNode(node);
    update();
};

void VNSpace::copy()
{
    while(!clipboard.isEmpty())
        delete clipboard.takeLast();

    foreach(QGraphicsItem *item, selectedItems())
    {
        if(item->type() != VNode::Type)continue;
        VNode *node = static_cast<VNode*>(item);
        DNode *copy = DNode::copy(node->data);
        FRG::CurrentProject->setNodePosition(copy, node->pos() - getSelectedItemsCenter());
        clipboard.append(copy);
    }
}

void VNSpace::cut()    
{
    copy();
    removeNode();
}

void VNSpace::paste()
{
    foreach(DNode *node, clipboard)
    {
        DNode *newNode = DNode::copy(node);
        FRG::SpaceDataInFocus->addNode(newNode);
        QPointF oldPosition = FRG::CurrentProject->getNodePosition(newNode);
        FRG::CurrentProject->setNodePosition(newNode, oldPosition + FRG::Space->getMousePos());
        addItem(newNode->createNodeVis());
    }
}

void VNSpace::addLink(VNSocket *final)
{
    DSocket::createLink(linksocket->data, final->data);
    leaveLinkNodeMode();
    updateLinks();
};

void VNSpace::removeLink(VNodeLink *link)
{
    removeItem(link);
    delete link;
    updateLinks();
}

void VNSpace::updateLinks()
{
    FRG::SpaceDataInFocus->cacheLinks();
    foreach(VNodeLink *vnlink, FRG::SpaceDataInFocus->getCachedLinksVis())
    {
        vnlink->updateLink(); 
        if(!items().contains(vnlink)) addItem(vnlink);
    }
}

void VNSpace::enterLinkNodeMode(VNSocket *socket)
{
	linkNodeMode = true;
    QPointF endpos;
    linksocket = socket;
    endpos = socket->parentItem()->pos() + socket->pos();
    newlink = new VNodeLink(endpos, endpos);
    addItem(newlink);
};

void VNSpace::leaveLinkNodeMode()
{
	if(!linkNodeMode)
        return;

    linkNodeMode = false;
    VNSocket *finalSocket;
    removeItem(newlink);
    linksocket = 0;
    newlink = 0;
};

bool VNSpace::isLinkNodeMode()
{
	return linkNodeMode;
}

void VNSpace::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mousePos = event->scenePos();
    if (isLinkNodeMode())
    {
        QPointF lpos;

        //snap to compatible socket
        QGraphicsItem *item = itemAt(event->scenePos());
        if(item && item->type() == VNSocket::Type)
        {
            VNSocket *vsocket = (VNSocket*)item;
            if(DSocket::isCompatible(linksocket->data, vsocket->data))
                lpos = item->pos() + item->parentItem()->pos();
            else
                lpos = event->scenePos();
        }
        else
            lpos = event->scenePos();
        newlink->setlink(lpos);
    }
    else
        updateLinks();
        QGraphicsScene::mouseMoveEvent(event);
    update();
};

void VNSpace::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    QGraphicsItem *item = itemAt(event->scenePos());
    if(!item || item->type() != VNSocket::Type)
        leaveLinkNodeMode();
    updateLinks();
    update();
};

void VNSpace::containerNode()    
{
    QPointF pos = getSelectedItemsCenter();
    DNode *node = DNode::buildContainerNode();
    addItem(node->createNodeVis());
    FRG::CurrentProject->setNodePosition(node, pos);
}

void VNSpace::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    mousePos = event->scenePos();
    QGraphicsItem *item = itemAt(mousePos);
    if (item)
        QGraphicsScene::contextMenuEvent(event);
    else
    {
        ContextAddMenu();
        NContextMenu->exec(event->screenPos());
    }
};

void VNSpace::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    DNode *dnode = 0;
    if(itemAt(event->scenePos()))
    {
        QGraphicsScene::dropEvent(event);
        return;
    }
    if (!event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")
        || nodelib->selectedItems().size() == 0)
        return;
    QTreeWidgetItem *node = nodelib->selectedItems().first();
    if (QFileInfo(node->text(1)).isFile())
    {
        QFileInfo file(node->text(1));
        dnode = DNode::dropNode(node->text(1));
    }
    else
    {
        if(node->parent()->parent()
            &&node->parent()->parent()->text(0) == "Build In")
        {
            switch(node->text(1).toInt())
            {
            case 1:
                dnode = BuildIn::surfaceInput(FRG::SpaceDataInFocus);
                break;
            case 2:
                dnode = BuildIn::displacementInput(FRG::SpaceDataInFocus);
                break;
            case 3:
                dnode = BuildIn::volumeInput(FRG::SpaceDataInFocus);
                break;
            case 4:
                dnode = BuildIn::lightInput(FRG::SpaceDataInFocus);
                break;
            case 5:
                dnode = BuildIn::surfaceOutput(FRG::SpaceDataInFocus);
                break;
            case 6:
                dnode = BuildIn::displacementOutput(FRG::SpaceDataInFocus);
                break;
            case 7:
                dnode = BuildIn::volumeOutput(FRG::SpaceDataInFocus);
                break;
            case 8:
                dnode = BuildIn::lightOutput(FRG::SpaceDataInFocus);
                break;
            case 9:
                dnode = BuildIn::MaddNode(FRG::SpaceDataInFocus);
                break;
            case 10:
                dnode = BuildIn::MSubNode(FRG::SpaceDataInFocus);
                break;
            case 11:
                dnode = BuildIn::MmultNode(FRG::SpaceDataInFocus);
                break;
            case 12:
                dnode = BuildIn::MdivNode(FRG::SpaceDataInFocus);
                break;
            case 13:
                dnode = BuildIn::MdotNode(FRG::SpaceDataInFocus);
                break;
            case 14:
                dnode = BuildIn::ContIfNode(FRG::SpaceDataInFocus);
                break;
            case 15:
                dnode = BuildIn::CgreaterNode(FRG::SpaceDataInFocus);
                break;
            case 16:
                dnode = BuildIn::CsmallerNode(FRG::SpaceDataInFocus);
                break;
            case 17:
                dnode = BuildIn::CeqNode(FRG::SpaceDataInFocus);
                break;
            case 18:
                dnode = BuildIn::CnotNode(FRG::SpaceDataInFocus);
                break;
            case 19:
                dnode = BuildIn::CandNode(FRG::SpaceDataInFocus);
                break;
            case 20:
                dnode = BuildIn::CorNode(FRG::SpaceDataInFocus);
                break;
            case 21:
                dnode = BuildIn::VColNode(FRG::SpaceDataInFocus);
                break;
            case 22:
                dnode = BuildIn::VStrNode(FRG::SpaceDataInFocus);
                break;
            case 23:
                dnode = BuildIn::VFlNode(FRG::SpaceDataInFocus);
                break;
            case 24:
                dnode = BuildIn::ContForNode(FRG::SpaceDataInFocus);
                break;
            case 25:
                dnode = BuildIn::ContWhileNode(FRG::SpaceDataInFocus);
                break;
            case 26:
                dnode = BuildIn::CLilluminate(FRG::SpaceDataInFocus);
                break;
            case 27:
                dnode = BuildIn::CLilluminance(FRG::SpaceDataInFocus);
                break;
            case 28:
                dnode = BuildIn::CLsolar(FRG::SpaceDataInFocus);
                break;
            case 29:
                dnode = BuildIn::CLgather(FRG::SpaceDataInFocus);
                break;
            }
        }
    }
    if(dnode)
    {
        addItem(dnode->createNodeVis());
        FRG::CurrentProject->setNodePosition(dnode, event->scenePos());
    }
}

void VNSpace::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if(itemAt(event->scenePos()))
    {
        QGraphicsScene::dragMoveEvent(event);
        return;
    }
    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
        event->acceptProposedAction();
}

void VNSpace::shownodelib()
{
    if (!nodelib)
    {
        nodelib = new NodeLib(mousePos);
        nodelib->setScene(this);
    }
    else
        nodelib->container->setPos(mousePos);
}

NodeLib* VNSpace::getNodeLib()
{
	return nodelib;
}

NewNodeEditor* VNSpace::getNewNodeEditor()
{
	return nodeedit;
}

void VNSpace::enterEditNameMode()
{
    editNameMode = true;
}

void VNSpace::leaveEditNameMode()
{
    editNameMode = false;
}

bool VNSpace::isEditNameMode()    
{
    return editNameMode;
}

void VNSpace::destroyContainer(QGraphicsItem *container)
{
    QGraphicsProxyWidget *contwidget = (QGraphicsProxyWidget *)container->childItems().first();
    contwidget->setParentItem(0);
    removeItem(container);
    if(contwidget->widget() == nodeedit)
    {
        delete nodeedit;
        nodeedit = 0;
    }
    else if (contwidget->widget() == nodelib)
    {
        delete nodelib;
        nodelib = 0;
    }
    delete container;
}

void VNSpace::moveIntoSpace(DNSpace *space)
{
    if(space == FRG::SpaceDataInFocus)
        return;
    //delete old visualization
    deleteSpaceVis();

    FRG::SpaceDataInFocus = space;

    //create new visual nodes
    createSpaceVis();
}

void VNSpace::createSpaceVis()
{
    //create nodeVis for new nodes
    foreach(DNode* node, FRG::SpaceDataInFocus->getNodes())
    {
        addItem(node->createNodeVis());
		node->getNodeVis()->setPos(FRG::CurrentProject->getNodePosition(node));
    }
    updateLinks();
}

void VNSpace::deleteSpaceVis()
{
    if(!FRG::SpaceDataInFocus)
        return;
    //delete all Links
    FRG::SpaceDataInFocus->clearLinksCache();

    //cache positions
    FRG::SpaceDataInFocus->cacheNodePositions();

    //delete all visual nodes
	if(!FRG::SpaceDataInFocus->getNodes().isEmpty())
	{
		foreach(DNode *node, FRG::SpaceDataInFocus->getNodes())
            node->deleteNodeVis();
	}
}

void VNSpace::createNode()
{
    nodeedit = new NewNodeEditor(mousePos);
    nodeedit->setScene(this);
};


QPointF VNSpace::getSelectedItemsCenter()
{
    QList<QGraphicsItem *> items = selectedItems();
    int minX = items.first()->pos().x();
    int minY = items.first()->pos().y();
    int maxX = items.first()->pos().x();
    int maxY = items.first()->pos().y();

    foreach(QGraphicsItem *item, items)
    {
        if (item->pos().x() < minX) minX = item->pos().x();
        if (item->pos().y() < minY) minY = item->pos().y();
        if (item->pos().x() > maxX) maxX = item->pos().x();
        if (item->pos().y() > maxY) maxY = item->pos().y();
    }
    float cx = minX + ((maxX - minX) / 2);
    float cy = minY + ((maxY - minY) / 2);

    return QPointF(cx, cy);
}

QPointF VNSpace::getMousePos()
{
		return mousePos;
}

QList<DNode*> VNSpace::selectedNodes()
{
	QList<DNode*>selNodes;
	foreach(QGraphicsItem *item, selectedItems())
    if (item->type() == VNode::Type)
    {
        VNode *vnode = (VNode*)item;
        selNodes.append(vnode->data);
    }
	return selNodes;
}

bool VNSpace::isSelected(VNode *node)    
{
    return selectedNodes().contains(node->data);
}

