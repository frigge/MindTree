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
#include "source/graphics/shaderpreview.h"

UndoRemoveNode::UndoRemoveNode(QList<DNode*>nodes)
    : nodes(nodes)
{
}

UndoRemoveNode::~UndoRemoveNode()
{
    while(!nodes.isEmpty())
        delete nodes.takeLast();
}

void UndoRemoveNode::undo()
{
    foreach(DNode *node, nodes)
    {
        FRG::SpaceDataInFocus->addNode(node);
        FRG::Space->addItem(node->createNodeVis());
    }
}

void UndoRemoveNode::redo()    
{
    FRG::Space->removeNode(nodes);
}

UndoDropNode::UndoDropNode(DNode *node)    
    : node(node)
{
}

void UndoDropNode::undo()    
{
    FRG::SpaceDataInFocus->unregisterNode(node);
}

void UndoDropNode::redo()    
{
   FRG::SpaceDataInFocus->addNode(node);
   FRG::Space->addItem(node->createNodeVis());
}

UndoLink::UndoLink(DNodeLink dnlink, DNodeLink oldLink)
    : dnlink(dnlink), oldLink(oldLink)
{
}

void UndoLink::undo()    
{
    if(oldLink.in && oldLink.out)
    {
        dnlink.in->addLink(oldLink.out);
        return;
    }

    DinSocket *redoDinSocket = dnlink.in;
    DoutSocket *redoDoutSocket = dnlink.out;
    if(dnlink.in->getVariable())
        redoDinSocket = static_cast<DinSocket*>(dnlink.in->getNode()->getVarSocket());
    if(dnlink.out->getVariable())
        redoDoutSocket = static_cast<DoutSocket*>(dnlink.out->getNode()->getVarSocket());
    dnlink.in->clearLink();
    dnlink.in = redoDinSocket;
    dnlink.out = redoDoutSocket;
}

void UndoLink::redo()    
{
    dnlink.in->addLink(dnlink.out);
}

UndoRemoveLink::UndoRemoveLink(DNodeLink dnlink)    
    : dnlink(dnlink)
{
}

void UndoRemoveLink::undo()    
{
    dnlink.in->addLink(dnlink.out);
}

void UndoRemoveLink::redo()    
{
    DinSocket *redoDinSocket = dnlink.in;
    DoutSocket *redoDoutSocket = dnlink.out;
    if(dnlink.in->getVariable())
        redoDinSocket = static_cast<DinSocket*>(dnlink.in->getNode()->getVarSocket());
    if(dnlink.out->getVariable())
        redoDoutSocket = static_cast<DoutSocket*>(dnlink.out->getNode()->getVarSocket());
    dnlink.in->clearLink();
    dnlink.in = redoDinSocket;
    dnlink.out = redoDoutSocket;
}

UndoMoveNode::UndoMoveNode(NodeList nodes)    
{
   foreach(DNode *node, nodes)
        nodePositions.insert(node, FRG::CurrentProject->getNodePosition(node)); 
}

void UndoMoveNode::undo()    
{
    foreach(DNode *node, nodePositions.keys())
    {
        FRG::CurrentProject->setNodePosition(node, nodePositions.value(node));
        QPointF newPos = FRG::CurrentProject->getNodePosition(node);
        nodePositions.remove(node);
        nodePositions.insert(node, newPos);
        node->getNodeVis()->setPos(newPos);
    }
}

void UndoMoveNode::redo()    
{
    undo();
}

UndoBuildContainer::UndoBuildContainer(ContainerNode *contnode)
    : contnode(contnode)
{
    foreach(DNode *node, contnode->getContainerData()->getNodes())
        if(node != contnode->getInputs()
            &&node != contnode->getOutputs())
            containerNodes.append(node);
            
    foreach(DNode* node, containerNodes)
        foreach(DinSocket *socket, node->getInSockets())
            if(socket->getCntdSocket()
                &&socket->getCntdSocket()->getNode() == contnode->getInputs())
                oldLinks.append(DNodeLink(socket, socket->getCntdSocket()));
                
    foreach(DoutSocket *socket, contnode->getOutSockets())
        oldLinks.append(socket->getLinks());
}

void UndoBuildContainer::undo()    
{
    DNode::unpackContainerNode(contnode);
    FRG::Space->updateLinks();
}

void UndoBuildContainer::redo()    
{
    FRG::Space->centerNodes(containerNodes);
    foreach(DNode* node, containerNodes)
    {
        FRG::SpaceDataInFocus->unregisterNode(node);
        contnode->getContainerData()->addNode(node);
    }

    foreach(DNodeLink dnlink, oldLinks)
        dnlink.in->addLink(dnlink.out);

    FRG::SpaceDataInFocus->addNode(contnode);
    VNode *nodeVis = contnode->createNodeVis();
    FRG::Space->addItem(nodeVis);
    nodeVis->setPos(contnode->getPos());
    FRG::Space->updateLinks();
}

ContainerCache::ContainerCache(ContainerNode *contnode)
    : contnode(contnode)
{
    foreach(DNode *node, contnode->getContainerData()->getNodes())
        if(node != contnode->getInputs()
            &&node != contnode->getOutputs())
            containerNodes.append(node);

    foreach(DNode* node, containerNodes)
        foreach(DinSocket *socket, node->getInSockets())
            if(socket->getCntdSocket()
                &&socket->getCntdSocket()->getNode() == contnode->getInputs())
                oldLinks.append(DNodeLink(socket, socket->getCntdSocket()));
                
    foreach(DoutSocket *socket, contnode->getOutSockets())
        oldLinks.append(socket->getLinks());
}    

UndoUnpackContainer::UndoUnpackContainer(QList<ContainerNode*> contnodes)
{
    foreach(ContainerNode *contnode, contnodes)
        containers.append(ContainerCache(contnode));
}

void UndoUnpackContainer::undo()    
{
    foreach(ContainerCache cache, containers)
    {
        FRG::Space->centerNodes(cache.containerNodes);
        foreach(DNode* node, cache.containerNodes)
            {
                FRG::SpaceDataInFocus->unregisterNode(node);
                cache.contnode->getContainerData()->addNode(node);
            }

        foreach(DNodeLink dnlink, cache.oldLinks)
            dnlink.in->addLink(dnlink.out);

        FRG::SpaceDataInFocus->addNode(cache.contnode);
        VNode *nodeVis = cache.contnode->createNodeVis();
        FRG::Space->addItem(nodeVis);
        nodeVis->setPos(cache.contnode->getPos());
    }
    FRG::Space->updateLinks();
}

void UndoUnpackContainer::redo()    
{
    foreach(ContainerCache cache, containers)
        DNode::unpackContainerNode(cache.contnode);

    FRG::Space->updateLinks();
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

void VNSpace::removeNode(QList<DNode*>nodeList)
{
    foreach(DNode *node, nodeList)
        FRG::SpaceDataInFocus->unregisterNode(node);
    updateLinks();
};

void VNSpace::cacheNodePositions(NodeList nodes)    
{
    FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoMoveNode(nodes));
    foreach(DNode *node, nodes)
        FRG::CurrentProject->setNodePosition(node, node->getNodeVis()->pos());
}

QPointF VNSpace::getCenter(QList<DNode*>nodes)    
{
    int minX = FRG::CurrentProject->getNodePosition(nodes.first()).x();
    int minY = FRG::CurrentProject->getNodePosition(nodes.first()).y();
    int maxX = FRG::CurrentProject->getNodePosition(nodes.first()).x();
    int maxY = FRG::CurrentProject->getNodePosition(nodes.first()).y();

    foreach(DNode* node, nodes)
    {
        QPointF pos = FRG::CurrentProject->getNodePosition(node);
        int x = pos.x();
        int y = pos.y();
        if (x < minX) minX = x;
        if (y < minY) minY = y;
        if (x > maxX) maxX = x;
        if (y > maxY) maxY = y;
    }
    return QPointF(minX + ((maxX - minX) / 2), minY + ((maxY - minY) / 2));
}

void VNSpace::centerNodes(QList<DNode*>nodes)    
{
    QPointF center = getCenter(nodes);
    foreach(DNode* node, nodes)
    {
        QPointF oldPos = FRG::CurrentProject->getNodePosition(node);
        FRG::CurrentProject->setNodePosition(node, oldPos - center);
    }
}

void VNSpace::copy()
{
    if(selectedNodes().isEmpty()) return;
    while(!nodeClipboard.isEmpty())
        delete nodeClipboard.takeLast();

    QList<DNode*>copies = DNode::copy(selectedNodes());
    centerNodes(copies);
    nodeClipboard.append(copies);
}

void VNSpace::cut()    
{
    copy();
    foreach(DNode *node, selectedNodes())
        FRG::SpaceDataInFocus->removeNode(node);
}

void VNSpace::paste()
{
    QList<DNode*>nodes = DNode::copy(nodeClipboard);
    foreach(DNode *node, nodes)
    {
        FRG::SpaceDataInFocus->addNode(node);
        QPointF oldPosition = FRG::CurrentProject->getNodePosition(node);
        FRG::CurrentProject->setNodePosition(node, oldPosition + FRG::Space->getMousePos());
        VNode *nodeVis = node->createNodeVis();
        addItem(nodeVis);
        nodeVis->setPos(FRG::CurrentProject->getNodePosition(node));
    }
}

void VNSpace::addLink(VNSocket *final)
{
    DinSocket *socketToBeLinked = 0;
    if(linksocket->data->getDir() == IN) socketToBeLinked = static_cast<DinSocket*>(linksocket->data);
    else socketToBeLinked = static_cast<DinSocket*>(final->data);
    DNodeLink olddnlink;
    if(socketToBeLinked->getCntdSocket())
    {
        olddnlink.in = socketToBeLinked;
        olddnlink.out = socketToBeLinked->getCntdSocket();
    }
    DNodeLink dnlink = DSocket::createLink(linksocket->data, final->data);
    leaveLinkNodeMode();
    updateLinks();
    FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoLink(dnlink, olddnlink));

    emit linkChanged();
};

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
    QGraphicsScene::mouseMoveEvent(event);
    updateLinks();
    update();
};

void VNSpace::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    QGraphicsItem *item = itemAt(event->scenePos());
    if(!item || item->type() != VNSocket::Type)
        leaveLinkNodeMode();
};

void VNSpace::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)    
{
    QGraphicsScene::mouseReleaseEvent(event);
    QList<DNode*> nodes = selectedNodes();
    if(nodes.isEmpty())
        return QGraphicsScene::mouseReleaseEvent(event);
    
    DNode *firstNode = nodes.first();
    if(firstNode->getNodeVis()->pos() != FRG::CurrentProject->getNodePosition(firstNode))
        cacheNodePositions(nodes);
}

void VNSpace::containerNode()    
{
    QPointF pos = getCenter(selectedNodes());
    DNode *node = DNode::buildContainerNode(selectedNodes());
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
    if (event->mimeData()->hasFormat("FRGShaderAuthor/nodefile"))
    {
        QString filename;
        QByteArray itemData = event->mimeData()->data("FRGShaderAuthor/nodefile");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream>>filename;
        dnode = DNode::dropNode(filename);
    }
    else if(event->mimeData()->hasFormat("FRGShaderAuthor/buildInNode"))
    {
        int buildInType;
        QByteArray itemData = event->mimeData()->data("FRGShaderAuthor/buildInNode");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream>>buildInType;
        switch(buildInType)
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
        case 30:
            dnode = new DShaderPreview();
            FRG::SpaceDataInFocus->addNode(dnode);
        }
    }
    if(dnode)
    {
        FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoDropNode(dnode));
        addItem(dnode->createNodeVis());
        FRG::CurrentProject->setNodePosition(dnode, event->scenePos());
        dnode->getNodeVis()->setPos(event->scenePos());
    }
}

void VNSpace::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    updateLinks();
    if(itemAt(event->scenePos()))
    {
        QGraphicsScene::dragMoveEvent(event);
        return;
    }
    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
        event->acceptProposedAction();
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
    
    //delete all visual nodes
    if(!FRG::SpaceDataInFocus->getNodes().isEmpty())
    {
        foreach(DNode *node, FRG::SpaceDataInFocus->getNodes())
            node->deleteNodeVis();
    }
}

QPointF VNSpace::getMousePos()
{
		return mousePos;
}

NodeList VNSpace::selectedNodes()
{
	NodeList selNodes;
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

void VNSpace::buildContainer()    
{
    NodeList selectedNodes = FRG::Space->selectedNodes();
    ContainerNode *contnode = DNode::buildContainerNode(selectedNodes);
    if(!contnode)
        return;
    FRG::Space->addItem(contnode->createNodeVis());
    contnode->getNodeVis()->setPos(FRG::CurrentProject->getNodePosition(contnode));
    contnode->getNodeVis()->setSelected(true);
    updateLinks();
    FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoBuildContainer(contnode));
}

void VNSpace::unpackContainer()    
{
    QList<ContainerNode*>contnodes;
    foreach(DNode *node, selectedNodes())
        if(node->isContainer())
            contnodes.append(static_cast<ContainerNode*>(node));

    FRG::SpaceDataInFocus->registerUndoRedoObject(new UndoUnpackContainer(contnodes));
    foreach(ContainerNode *node, contnodes)
        DNode::unpackContainerNode(node);

    updateLinks();
}
