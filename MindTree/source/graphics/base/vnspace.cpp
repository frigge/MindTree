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


#include "QGraphicsSceneMouseEvent"
#include "QKeyEvent"

#include "iostream"

#include "data/frg.h"
#include "source/graphics/base/mindtree_mainwindow.h"
#include "data/project.h"
#include "data/nodes/buildin_nodes.h"
#include "data/nodes/node_db.h"
#include "data/signal.h"
#include "source/graphics/gui.h"
#include "vnspace.h"
//#include "source/graphics/shaderpreview.h"

using namespace MindTree;

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
        //spaceData->addNode(node);
    }
}

void UndoRemoveNode::redo()    
{
    //FRG::Space->removeNode(nodes);
}

UndoDropNode::UndoDropNode(DNode *node)    
    : node(node)
{
}

void UndoDropNode::undo()    
{
    //spaceData->unregisterNode(node);
}

void UndoDropNode::redo()    
{
   //spaceData->addNode(node);
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
        redoDinSocket = const_cast<DinSocket*>(dnlink.in->getNode()->getVarSocket()->toIn());
    if(dnlink.out->getVariable())
        redoDoutSocket = const_cast<DoutSocket*>(dnlink.out->getNode()->getVarSocket()->toOut());
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
        redoDinSocket = const_cast<DinSocket*>(dnlink.in->getNode()->getVarSocket()->toIn());
    if(dnlink.out->getVariable())
        redoDoutSocket = const_cast<DoutSocket*>(dnlink.out->getNode()->getVarSocket()->toOut());
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
        //node->getNodeVis()->setPos(newPos);
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
    //DNode::unpackContainerNode(contnode);
}

void UndoBuildContainer::redo()    
{
    //FRG::Space->centerNodes(containerNodes);
    foreach(DNode* node, containerNodes)
    {
        //spaceData->unregisterNode(node);
        contnode->getContainerData()->addNode(node);
    }

    foreach(DNodeLink dnlink, oldLinks)
        dnlink.in->addLink(dnlink.out);

    //spaceData->addNode(contnode);
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
        //FRG::Space->centerNodes(cache.containerNodes);
        foreach(DNode* node, cache.containerNodes)
            {
                //spaceData->unregisterNode(node);
                cache.contnode->getContainerData()->addNode(node);
            }

        foreach(DNodeLink dnlink, cache.oldLinks)
            dnlink.in->addLink(dnlink.out);

        //spaceData->addNode(cache.contnode);
    }
}

void UndoUnpackContainer::redo()    
{
    //foreach(ContainerCache cache, containers)
        //DNode::unpackContainerNode(cache.contnode);
}

VNSpace::VNSpace()
    : newlink(0),
      nodelib(0),
      nodeedit(0),
      editNameMode(false),
      linkNodeMode(false),
      linksocket(0),
      spaceData(0)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
    //qreal space_width, space_height;
    //if (itemsBoundingRect().width() < 10000)
    //    space_width = 10000;
    //else
    //    space_width = itemsBoundingRect().width() + 100;
    //if (itemsBoundingRect().height() < 10000)
    //    space_height = 10000;
    //else
    //    space_height = itemsBoundingRect().height() + 100;
    //QRectF space_rect(0-(space_width/2), 0-(space_height/2), space_width, space_height);
    //setSceneRect(space_rect);
    //setSceneRect(0, 0, 10000, 10000);
    connect(FRG::Author, SIGNAL(projectChanged()), this, SLOT(moveIntoRootSpace()));
    connectCallbacks();
    NodeDesigner::init();
    //MindTree::Signal::getHandler<>().add("registerPyDataTypes", []{
    //    BPy::class_<PyNodeGraphics>("NodeItem");
    //}).detach();
}

void VNSpace::connectCallbacks()    
{
    MindTree::Signal::getHandler<MindTree::DNode*>().add(
            "void MindTree::DNSpace::addNode(MindTree::DNode*)", 
            [this](DNode *node){ this->createVis(node);}
            ).detach();
    MindTree::Signal::getHandler<MindTree::DinSocket*>().add(
            "void MindTree::DinSocket::setCntdSocket(MindTree::DoutSocket*)",
            [this](DinSocket* socket){this->createLinkVis(DNodeLink(socket, socket->getCntdSocket()));}
            ).detach();
}

void VNSpace::expandTop(float val)    
{
    std::cout<< "expanding top by " << val << std::endl;
    QRectF sr = sceneRect();
    sr.setHeight(sceneRect().height() + val); 
    foreach(DNode *node, spaceData->getNodes()){
        QPointF pos = FRG::CurrentProject->getNodePosition(node);
        FRG::CurrentProject->setNodePosition(node, QPointF(pos.x(), pos.y() + val));
    }
    setSceneRect(sr);
}

void VNSpace::shrinkTop(float val)    
{
    expandTop(val);
    //std::cout<< "shrinking top by " << val << std::endl;
    //QRectF sr = sceneRect();
    //sr.setHeight(sceneRect().height() - val); 
    //foreach(DNode *node, spaceData->getNodes()){
    //    QPointF pos = FRG::CurrentProject->getNodePosition(node);
    //    FRG::CurrentProject->setNodePosition(node, QPointF(pos.x(), pos.y() - val));
    //}
    //setSceneRect(sr);
}

void VNSpace::expandBottom(float val)    
{
    QRectF sr = sceneRect();
    sr.setHeight(sceneRect().height() + val); 
    setSceneRect(sr);
}

void VNSpace::shrinkBottom(float val)    
{
    expandBottom(val);
    //std::cout<< "shrinking bottom by " << val << std::endl;
    //QRectF sr = sceneRect();
    //sr.setHeight(sceneRect().height() - val); 
    //setSceneRect(sr);
}

void VNSpace::expandLeft(float val)    
{
    QRectF sr = sceneRect();
    sr.setWidth(sceneRect().width() + val); 
    foreach(DNode *node, spaceData->getNodes()){
        QPointF pos = FRG::CurrentProject->getNodePosition(node);
        FRG::CurrentProject->setNodePosition(node, QPointF(pos.x() + val, pos.y()));
    }
    setSceneRect(sr);
}

void VNSpace::shrinkLeft(float val)    
{
    expandLeft(val);
    //std::cout<< "shrinking left by " << val << std::endl;
    //QRectF sr = sceneRect();
    //sr.setWidth(sceneRect().width() - val); 
    //foreach(DNode *node, spaceData->getNodes()){
    //    QPointF pos = FRG::CurrentProject->getNodePosition(node);
    //    FRG::CurrentProject->setNodePosition(node, QPointF(pos.x() - val, pos.y()));
    //}
    //setSceneRect(sr);
}

void VNSpace::expandRight(float val)    
{
    std::cout<< "expanding right by " << val << std::endl;
    QRectF sr = sceneRect();
    sr.setWidth(sceneRect().width() + val); 
    setSceneRect(sr);
}

void VNSpace::shrinkRight(float val)    
{
    expandRight(val);
    //std::cout<< "shrinking right by " << val << std::endl;
    //QRectF sr = sceneRect();
    //sr.setWidth(sceneRect().width() - val); 
    //setSceneRect(sr);
}

void VNSpace::ContextAddMenu()
{
    NContextMenu = new QMenu;
};

void VNSpace::removeNode(QList<DNode*>nodeList)
{
    foreach(DNode *node, nodeList)
    {
        if(node->getNodeType() == INSOCKETS
            ||node->getNodeType() == OUTSOCKETS
            ||node->getNodeType() == LOOPINSOCKETS
            ||node->getNodeType() == LOOPOUTSOCKETS)
            continue;
        QGraphicsItem *vn = visibleNodes.take(node);
        removeItem(vn);
        delete vn;
        delete node;
    }
};

void VNSpace::regLink(VNodeLink *l)    
{
    links.append(l); 
    addItem(l);
}

void VNSpace::rmLink(VNodeLink *l)    
{
    links.removeAll(l);
}

void VNSpace::cacheNodePositions(NodeList nodes)    
{
    spaceData->registerUndoRedoObject(new UndoMoveNode(nodes));
   // foreach(DNode *node, nodes)
   //     FRG::CurrentProject->setNodePosition(node, node->getNodeVis()->pos());
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
        spaceData->removeNode(node);
}

void VNSpace::paste()
{
    QList<DNode*>nodes = DNode::copy(nodeClipboard);
    foreach(DNode *node, nodes)
    {
        spaceData->addNode(node);
        QPointF oldPosition = FRG::CurrentProject->getNodePosition(node);
        //FRG::CurrentProject->setNodePosition(node, oldPosition + FRG::Space->getMousePos());
    }
}

void VNSpace::addLink(VNSocket *final)
{
    DNodeLink dnlink;
    if(linksocket)
        dnlink = DNodeLink(DSocket::createLink(linksocket->getData(), final->getData()));
    newlink->setData(dnlink);
    if(final->getData()->getDir() != IN) {
        newlink->setRoute(final, newlink->getStart());
    }
    else
        newlink->setRoute(newlink->getStart(), final);

    //emit linkChanged(dnlink.out->getNode());
};

void VNSpace::updateLinks()
{
}

void VNSpace::enterLinkNodeMode(VNSocket *socket)
{
	linkNodeMode = true;
    linksocket = socket;
    newlink = new VNodeLink();
    addItem(newlink);
    newlink->setTemp(socket);
};

VNSocket* VNSpace::getLinkSocket()    
{
    return linksocket;
}

void VNSpace::leaveLinkNodeMode()
{
	if(!linkNodeMode)
        return;

    linkNodeMode = false;
    linksocket = 0;
    delete newlink;
    newlink = 0;
};

void VNSpace::cancelLinkNodeMode()    
{
    leaveLinkNodeMode();
}

bool VNSpace::isLinkNodeMode()
{
	return linkNodeMode;
}

void VNSpace::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)    
{
    if(linkNodeMode) cancelLinkNodeMode();
    return QGraphicsScene::mouseReleaseEvent(event);
}

void VNSpace::containerNode()    
{
    QPointF pos = getCenter(selectedNodes());
    //DNode *node = DNode::buildContainerNode(selectedNodes());
}

void VNSpace::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    mousePos = event->scenePos();
    QGraphicsItem *item = itemAt(mousePos);
    if (item)
        QGraphicsScene::contextMenuEvent(event);
    else {
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
        leaveLinkNodeMode();
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
        MindTree::AbstractNodeFactory *factory = FRG::lib->getItem(buildInType)->factory;
        dnode = (*factory)();
        //dnode = (*FRG::lib->getItem(buildInType)->factory)();
    }
    else if(isLinkNodeMode())
        cancelLinkNodeMode();
    if(dnode)
    {
        spaceData->addNode(dnode);
        spaceData->registerUndoRedoObject(new UndoDropNode(dnode));
        dnode->setPos(event->scenePos());

        emit nodeDroped(dnode);
        //automatically connect nodes if CTRL is pressed while dropping
        if(event->modifiers() & Qt::ControlModifier
            &&!selectedNodes().isEmpty())
        {
            foreach(DNode *node, selectedNodes())
            {
                if(event->scenePos().x() < FRG::CurrentProject->getNodePosition(node).x())
                {
                    if(node->getInSockets().isEmpty() || dnode->getOutSockets().isEmpty())
                        continue;
                    DoutSocket *out = dnode->getOutSockets().first();
                    foreach(DinSocket *socket, node->getInSockets())
                        if(!socket->getCntdSocket()
                            && DSocket::isCompatible(socket, out))
                        {
                            DSocket::createLink(socket, out);
                            break;
                        }
                }
                else
                {
                    if(dnode->getInSockets().isEmpty() || node->getOutSockets().isEmpty())
                        continue;
                    DoutSocket *out = node->getOutSockets().first();
                    foreach(DinSocket *socket, dnode->getInSockets())
                        if(!socket->getCntdSocket()
                            && DSocket::isCompatible(socket, out))
                        {
                            DSocket::createLink(socket, out);
                            break;
                        }
                }
            }
        }
        foreach(QGraphicsItem *item, selectedItems())
            if(item->type() == VNode::Type
                && ((VNode*)item)->data == dnode)
                item->setSelected(true);
            else
                item->setSelected(false);
    }
}

void VNSpace::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    update();
    mousePos = event->scenePos();
    if (isLinkNodeMode())
    {
        QPointF lpos;

        //snap to compatible socket
        QGraphicsItem *item = itemAt(event->scenePos());
        if(item && item->type() == VNSocket::Type) {
            VNSocket *socket = (VNSocket*)item;
            if(DSocket::isCompatible(linksocket->getData(), socket->getData()))
                lpos = item->pos() + item->parentItem()->pos();
            else
                lpos = event->scenePos();
        }
        else
            lpos = event->scenePos();
        newlink->setLink(lpos);
    }

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
    if(space == spaceData)
        return;
    spaceData = space;
    //delete old visualization
    deleteSpaceVis();

    //connect(space, SIGNAL(nodeCreated(DNode*)), this, SLOT(createVis(DNode*)));

    //create new visual nodes
    createSpaceVis();
}

void VNSpace::moveIntoRootSpace()    
{
    moveIntoSpace(FRG::CurrentProject->getRootSpace()); 
}

void VNSpace::createSpaceVis()
{
    //create nodeVis for new nodes
    foreach(DNode* node, spaceData->getNodes()) {
        createVis(node);
    }
    emit linkChanged();

    createLinkVis();
    createInfoBoxes();
}

void VNSpace::createVis(DNode* node)    
{
    //QGraphicsItem *vnode = NodeDesigner::create(node->getType());
    VNode *vnode = new VNode(node);
    visibleNodes.insert(node, vnode);
    addItem(vnode);
}

void VNSpace::deleteNodeVis(DNode* node)    
{
    QGraphicsItem *vn = visibleNodes.take(node);
    removeItem(vn);
    delete vn;
}

void VNSpace::createInfoBoxes()    
{
    foreach(DInfoBox *box, spaceData->getInfoBoxes()) 
        box->getVis();
}

void VNSpace::createLinkVis(DNodeLink link)    
{
    //auto vlink = new VNodeLink(new DNodeLink(link));
    //auto out = visibleNodes.value(link.out->getNode())->getSocketVis(link.out);
    //auto in = visibleNodes.value(link.in->getNode())->getSocketVis(link.in);
    //vlink->setRoute(out, in);
    //regLink(vlink);
}

void VNSpace::createLinkVis()    
{
    //VNodeLink *vnlink = 0;
    //foreach(DNode *node, spaceData->getNodes()) {
    //    VNode *vn = visibleNodes.value(node);
    //    foreach(DinSocket *socket, node->getInSockets()) {
    //        VNSocket *vns = vn->getSocketVis(socket);
    //        if(socket->getArray()){
    //            foreach(DoutSocket *os, ((DAInSocket*)socket)->getLinks()){
    //                vnlink = new VNodeLink(new DNodeLink(socket, os));
    //            }
    //        } else {
    //            vnlink = new VNodeLink(new DNodeLink(socket, socket->getCntdSocket()));
    //            DoutSocket *cntdSocket = socket->getCntdSocket();
    //            VNode *cntdNodeVis = visibleNodes.value(cntdSocket->getNode());
    //            vnlink->setRoute(cntdNodeVis->getSocketVis(cntdSocket), vns);
    //        }
    //    }
    //}
}

void VNSpace::deleteSpaceVis()
{
    if(!spaceData)
        return;
    //delete all Links
    deleteLinkVis();
    deleteInfoBoxes();

    //delete all visual nodes
    if(!spaceData->getNodes().isEmpty()) {
        foreach(QGraphicsItem *vnode, visibleNodes.values())
            delete vnode;
        visibleNodes.clear();
    }
}

void VNSpace::deleteInfoBoxes()    
{
    foreach(DInfoBox *box, spaceData->getInfoBoxes()){
        box->deleteVis();
    }
}

void VNSpace::deleteLinkVis()    
{
    foreach(VNodeLink *link, links)
        delete link;
    links.clear();
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
    NodeList nodes = selectedNodes();
    //ContainerNode *contnode = DNode::buildContainerNode(nodes);
    //if(!contnode)
    //    return;
    //emit nodeDroped(contnode);
    //spaceData->registerUndoRedoObject(new UndoBuildContainer(contnode));
}

void VNSpace::unpackContainer()    
{
    QList<ContainerNode*>contnodes;
    foreach(DNode *node, selectedNodes())
        if(node->isContainer())
            contnodes.append(static_cast<ContainerNode*>(node));

    spaceData->registerUndoRedoObject(new UndoUnpackContainer(contnodes));
    //foreach(ContainerNode *node, contnodes)
    //    DNode::unpackContainerNode(node);
}
