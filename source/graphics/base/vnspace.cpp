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
#include "source/data/base/project.h"
#include "source/data/nodes/buildin_nodes.h"

VNSpace::VNSpace()
{
    newlink = 0;
    nodelib = 0;
    nodeedit = 0;
    editNameMode = false;
    qreal space_width, space_height;
    if (itemsBoundingRect().width() < 10000)
        space_width = 10000;
    else
        space_width = itemsBoundingRect().width();
    if (itemsBoundingRect().height() < 10000)
        space_height = 10000;
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
    foreach(QGraphicsItem *item, selectedItems())
    {
        VNode *node = (VNode*)item;
        if (node && node->type() == VNode::Type)
        {
            node->data->clearSocketLinks();
            removeItem(item);
            delete node->data;
        }
    }
    update();
};

void VNSpace::removeSelectedNodes()
{
    removeNode();
}

void VNSpace::addLink(NodeLink *nlink, VNSocket *final)
{
    nlink->setlink(linksocket->data, final->data);
    cachedLinks.insert(nlink->inSocket, nlink);
    if(!items().contains(nlink)) addItem(nlink);
    connect(nlink, SIGNAL(removeLink(NodeLink*)), this, SLOT(removeLink(NodeLink*)));
};

void VNSpace::addLink(NodeLink *nlink)
{
    addItem(nlink);
    cachedLinks.insert(nlink->inSocket, nlink);
    connect(nlink, SIGNAL(removeLink(NodeLink*)), this, SLOT(removeLink(NodeLink*)));
}

void VNSpace::removeLink(NodeLink *link)
{
    cachedLinks.remove(link->inSocket);
    removeItem(link);
    disconnect(link, SIGNAL(removeLink(NodeLink*)), this, SLOT(removeLink(NodeLink*)));
    delete link;
    update();
}

void VNSpace::updateLinks()
{
    foreach(DNSocket *socket, cachedLinks.keys())
    {
        if(!spaceData->getNodes().contains(socket->node)
                ||socket->cntdSockets.isEmpty())
        {
            removeItem(cachedLinks.value(socket));
            delete cachedLinks.value(socket);
        }
        cachedLinks.value(socket)->updateLink();
    }
};

void VNSpace::enterlinkNodeMode(VNSocket *socket)
{
	linkNodeMode = true;
    QPointF endpos;
    linksocket = socket;
    endpos = socket->parentItem()->pos() + socket->pos() + QPointF(-15, 22);
    newlink = new NodeLink(endpos, endpos);
    addItem(newlink);
};

void VNSpace::leavelinkNodeMode(QPointF finalpos)
{
	if(linkNodeMode)
    {
		linkNodeMode = false;
        VNSocket *finalSocket;
        finalSocket = 0;
        finalSocket = dynamic_cast<VNSocket*>(itemAt(finalpos));
        if (NodeLink::isCompatible(linksocket, finalSocket))
            addLink(newlink, finalSocket);
        else
            removeItem(newlink);

    linksocket = 0;
    newlink = 0;
    }
};

bool VNSpace::isLinkNodeMode()
{
	return linkNodeMode;
}

void VNSpace::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mousePos = event->scenePos();
    if (newlink != 0)
    {
        QPointF lpos;
        QGraphicsItem *item = itemAt(event->scenePos());
        if(item && item->type() == VNSocket::Type && NodeLink::isCompatible(linksocket, (VNSocket*)item))
            lpos = item->pos() + item->parentItem()->pos() + QPointF(-15, 22);
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
    if(!itemAt(event->scenePos())
        || itemAt(event->scenePos())->type() != VNSocket::Type)
            leavelinkNodeMode(event->scenePos());
    QGraphicsScene::mousePressEvent(event);
    update();
};

void VNSpace::keyPressEvent(QKeyEvent *event)
{
    if(!editNameMode)
    {
        switch(event->key())
        {
        case Qt::Key_Space:
            shownodelib();
            break;
        case Qt::Key_X:
            removeNode();
            break;
        case Qt::Key_C:
            if(selectedItems().size()>0)DNode::buildContainerNode(getSelectedItemsCenter());
            break;
        }
    }
    QGraphicsScene::keyPressEvent(event);
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
        DNode *newnode;
        newnode = DNode::dropNode(node->text(1));

        showNode(newnode);
        newnode->nodeVis->setPos(event->scenePos());
    }
    else
    {
        if(node->parent()->parent()
            &&node->parent()->parent()->text(0) == "Build In")
        {
            mousePos = event->scenePos();
            switch(node->text(1).toInt())
            {
            case 1:
                BuildIn::surfaceInput(spaceData);
                break;
            case 2:
                BuildIn::displacementInput(spaceData);
                break;
            case 3:
                BuildIn::volumeInput(spaceData);
                break;
            case 4:
                BuildIn::lightInput(spaceData);
                break;
            case 5:
                BuildIn::surfaceOutput(spaceData);
                break;
            case 6:
                BuildIn::displacementOutput(spaceData);
                break;
            case 7:
                BuildIn::volumeOutput(spaceData);
                break;
            case 8:
                BuildIn::lightOutput(spaceData);
                break;
            case 9:
                BuildIn::MaddNode(spaceData);
                break;
            case 10:
                BuildIn::MSubNode(spaceData);
                break;
            case 11:
                BuildIn::MmultNode(spaceData);
                break;
            case 12:
                BuildIn::MdivNode(spaceData);
                break;
            case 13:
                BuildIn::MdotNode(spaceData);
                break;
            case 14:
                BuildIn::ContIfNode(spaceData);
                break;
            case 15:
                BuildIn::CgreaterNode(spaceData);
                break;
            case 16:
                BuildIn::CsmallerNode(spaceData);
                break;
            case 17:
                BuildIn::CeqNode(spaceData);
                break;
            case 18:
                BuildIn::CnotNode(spaceData);
                break;
            case 19:
                BuildIn::CandNode(spaceData);
                break;
            case 20:
                BuildIn::CorNode(spaceData);
                break;
            case 21:
                BuildIn::VColNode(spaceData);
                break;
            case 22:
                BuildIn::VStrNode(spaceData);
                break;
            case 23:
                BuildIn::VFlNode(spaceData);
                break;
            case 24:
                BuildIn::ContForNode(spaceData);
                break;
            case 25:
                BuildIn::ContWhileNode(spaceData);
                break;
            case 26:
                BuildIn::CLilluminate(spaceData);
                break;
            case 27:
                BuildIn::CLilluminance(spaceData);
                break;
            case 28:
                BuildIn::CLsolar(spaceData);
                break;
            case 29:
                BuildIn::CLgather(spaceData);
                break;
            }
        }

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
    //delete old visualization
    deleteSpaceVis();

    spaceData = space;
	FRG::project->setCurrentSpace(space);

    //create new visual nodes
    createSpaceVis();
}

void VNSpace::createSpaceVis()
{
    //create nodeVis for new nodes
    foreach(DNode* node, spaceData->getNodes())
    {
        if(!node->nodeVis)
            node->createNodeVis();
        addItem(node->nodeVis);
    }

    //cache links of visual nodes
    cacheLinks();
}

void VNSpace::cacheLinks()
{
    foreach(DNode *node, spaceData->getNodes())
    {
        foreach(DNSocket *socket, *node->N_inSockets)
        {
            if(!cachedLinks.contains(socket))
            {
                NodeLink *nlink = new NodeLink;
                nlink->setlink(socket, socket->cntdSockets.first());
                addLink(nlink);
            }
        }
    }
}

void VNSpace::deleteSpaceVis()
{
    //delete all Links
    foreach(NodeLink *link, cachedLinks.values())
    {
        removeItem(link);
        delete link;
    }

    //delete all visual nodes
    foreach(DNode *node, spaceData->getNodes())
    {
        removeItem(node->nodeVis);
        delete node->nodeVis;
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

void VNSpace::showNode(DNode *node)
{
    if(!node->nodeVis)node->createNodeVis();
    addItem(node->nodeVis);
}
QPointF VNSpace::getMousePos()
{
		return mousePos;
}

