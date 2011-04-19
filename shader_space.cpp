#include "QMouseEvent"
#include "QGraphicsSceneMouseEvent"
#include "QMenu"
#include "QAction"
#include "QObject"
#include "QTreeWidget"
#include "QGraphicsProxyWidget"
#include "QGraphicsItem"
#include "QLineEdit"

#include "newnodeeditor.h"
#include "shader_space.h"
#include "node.h"
#include "nodelink.h"
#include "scenewidgetcontainer.h"
#include "nodelib.h"
#include "frg_shader_author.h"
#include "buildin_nodes.h"

#include "math.h"

Shader_Space::Shader_Space()
{
    nodecnt = 0;
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
};

void Shader_Space::ContextAddMenu()
{
    NContextMenu = new QMenu;

    QAction *newNodeAction = NContextMenu->addAction("New Node");
    connect(newNodeAction, SIGNAL(triggered()), this, SLOT(createNode()));
};

void Shader_Space::removeNode(bool keeplinks)
{
    QList<QGraphicsItem *> items = selectedItems();
    foreach(QGraphicsItem *item, items)
    {
        Node *node = dynamic_cast<Node *>(item);
        if (node && node->type() == Node::Type)
        {
            node->clearSocketLinks(keeplinks);
            removeItem(item);
            delete node;
            nodecnt--;
        }
    }
    update();
};

void Shader_Space::removeSelectedNodes()
{
    removeNode(false);
}

void Shader_Space::addLink(NodeLink *nlink, NSocket *final)
{
    nlink->setlink(linksocket, final);
    links.append(nlink);
    if(!items().contains(nlink)) addItem(nlink);
    connect(nlink, SIGNAL(removeLink(NodeLink*)), this, SLOT(removeLink(NodeLink*)));
};

void Shader_Space::addLink(NodeLink *nlink)
{
    nlink->inSocket->addLink(nlink);
    nlink->outSocket->addLink(nlink);
    addItem(nlink);
    links.append(nlink);
    connect(nlink, SIGNAL(removeLink(NodeLink*)), this, SLOT(removeLink(NodeLink*)));
}

void Shader_Space::moveLink(NodeLink *nlink)
{
    Shader_Space *oldspace = (Shader_Space*)nlink->scene();
    oldspace->disconnect(nlink, SIGNAL(removeLink(NodeLink*)), oldspace, SLOT(removeLink(NodeLink*)));
    oldspace->links.removeAll(nlink);
    links.append(nlink);
    addItem(nlink);
    connect(nlink, SIGNAL(removeLink(NodeLink*)), this, SLOT(removeLink(NodeLink*)));
}


void Shader_Space::removeLink(NodeLink *link)
{
    links.removeAll(link);
    if(link->inSocket)link->inSocket->removeLink(link);
    if(link->outSocket)link->outSocket->removeLink(link);
    removeItem(link);
    disconnect(link, SIGNAL(removeLink(NodeLink*)), this, SLOT(removeLink(NodeLink*)));
    delete link;

    update();
}

void Shader_Space::updateLinks()
{
    foreach(NodeLink *link, links)
        link->updateLink();
};

void Shader_Space::enterlinkNodeMode(NSocket *socket)
{
    QPointF endpos;
    linksocket = socket;
    endpos = socket->parentItem()->pos() + socket->pos() + QPointF(-15, 22);
    newlink = new NodeLink(endpos, endpos);
    addItem(newlink);
};

void Shader_Space::leavelinkNodeMode(QPointF finalpos)
{
    if (newlink != 0)
    {
        NSocket *finalSocket;
        finalSocket = 0;
        finalSocket = dynamic_cast<NSocket*>(itemAt(finalpos));
        if (NodeLink::isCompatible(linksocket, finalSocket))
            addLink(newlink, finalSocket);
        else
            removeItem(newlink);

    linksocket = 0;
    newlink = 0;
    }
};

void Shader_Space::changeScene(QGraphicsScene *scene)
{
    Shader_View *view = dynamic_cast<Shader_View *>(views().first());
    frg_Shader_Author *author = dynamic_cast<frg_Shader_Author *>(view->frgauthor);
    view->setScene(scene);
    author->moveIntoSpace(scene);
    author->updateToolBar();
}

void Shader_Space::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mousePos = event->scenePos();
    if (newlink != 0)
    {
        QPointF lpos;
        QGraphicsItem *item = itemAt(event->scenePos());
        if(item && item->type() == NSocket::Type && NodeLink::isCompatible(linksocket, (NSocket*)item))
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

void Shader_Space::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!itemAt(event->scenePos())
        || itemAt(event->scenePos())->type() != NSocket::Type)
            leavelinkNodeMode(event->scenePos());
    QGraphicsScene::mousePressEvent(event);
    update();
};

void Shader_Space::keyPressEvent(QKeyEvent *event)
{
    if(!editNameMode)
    {
        switch(event->key())
        {
        case Qt::Key_Space:
            shownodelib();
            break;
        case Qt::Key_X:
            removeNode(false);
            break;
        case Qt::Key_C:
            if(selectedItems().size()>0)buildContainerNode();
            break;
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void Shader_Space::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
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

void Shader_Space::dropEvent(QGraphicsSceneDragDropEvent *event)
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
        Node *newnode;
        newnode = dropNode(node->text(1));

        addNode(newnode);
        newnode->setPos(event->scenePos());
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
                BuildIn::surfaceInput(this);
                break;
            case 2:
                BuildIn::displacementInput(this);
                break;
            case 3:
                BuildIn::volumeInput(this);
                break;
            case 4:
                BuildIn::lightInput(this);
                break;
            case 5:
                BuildIn::surfaceOutput(this);
                break;
            case 6:
                BuildIn::displacementOutput(this);
                break;
            case 7:
                BuildIn::volumeOutput(this);
                break;
            case 8:
                BuildIn::lightOutput(this);
                break;
            case 9:
                BuildIn::MaddNode(this);
                break;
            case 10:
                BuildIn::MSubNode(this);
                break;
            case 11:
                BuildIn::MmultNode(this);
                break;
            case 12:
                BuildIn::MdivNode(this);
                break;
            case 13:
                BuildIn::MdotNode(this);
                break;
            case 14:
                BuildIn::ContIfNode(this);
                break;
            case 15:
                BuildIn::CgreaterNode(this);
                break;
            case 16:
                BuildIn::CsmallerNode(this);
                break;
            case 17:
                BuildIn::CeqNode(this);
                break;
            case 18:
                BuildIn::CnotNode(this);
                break;
            case 19:
                BuildIn::CandNode(this);
                break;
            case 20:
                BuildIn::CorNode(this);
                break;
            case 21:
                BuildIn::VColNode(this);
                break;
            case 22:
                BuildIn::VStrNode(this);
                break;
            case 23:
                BuildIn::VFlNode(this);
                break;
            case 24:
                BuildIn::ContForNode(this);
                break;
            case 25:
                BuildIn::ContWhileNode(this);
            }
        }

    }
}

void Shader_Space::addNode(Node *node)
{
    addItem(node);
    nodecnt++;
}

QPointF Shader_Space::getSelectedItemsCenter()
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

QList<Node *> Shader_Space::copySelectedNodes(bool centered, QHash<NSocket *, NSocket *> *socketmapping)
{
    QList<Node*> newnodes;
    foreach(QGraphicsItem *item, selectedItems())
    {
        Node *node = (Node *)item;
        Node *newnode = node->copyNode(socketmapping);
        if (centered)
        {
            QPointF center = getSelectedItemsCenter();
            int x, y;
            x = node->pos().x() - abs(center.x());
            y = node->pos().y() - abs(center.y());
            newnode->setPos(x, y);
        }
        newnodes.append(newnode);
    }
    return newnodes;
}

void Shader_Space::buildContainerNode()
{
    ContainerNode *contnode = new ContainerNode("");
    Shader_Space *cont_space = (Shader_Space*)contnode->ContainerData;
    contnode->setNodeName("New Node");

    addNode(contnode);
    contnode->setPos(getSelectedItemsCenter());

    QList<NodeLink *> ins = getContainerInLinks();
    QList<NodeLink *> outs = getContainerOutLinks();
    QList<NodeLink *> cDataLinks = getContainerDataLinks();

    foreach(QGraphicsItem *item, selectedItems())
    {
        if(item->type()==Node::Type)
        {
            nodecnt--;
            Node *node = (Node*)item;
            cont_space->addNode(node);
        }
    }
    NodeLink *newlink;

    foreach(NodeLink *nlink, ins)
    {
        newlink = new NodeLink;
        newlink->inSocket = nlink->inSocket;
        newlink->outSocket = contnode->inSocketNode->varsocket;
        cont_space->addLink(newlink);

        nlink->setlink(contnode->socket_map[contnode->inSocketNode->lastsocket], nlink->outSocket);
    }

    foreach(NodeLink *nlink, outs)
    {
        newlink = new NodeLink;
        newlink->inSocket = contnode->outSocketNode->varsocket;
        newlink->outSocket = nlink->outSocket;
        cont_space->addLink(newlink);

        nlink->setlink(nlink->inSocket, contnode->socket_map[contnode->outSocketNode->lastsocket]);
    }

    foreach(NodeLink *nlink, cDataLinks)
        cont_space->moveLink(nlink);
}

QList<NodeLink *> Shader_Space::getContainerInLinks()
{
    QList<QGraphicsItem *> items = selectedItems();

    QList<NodeLink *> ins;

    foreach(NodeLink *nlink, links)
        if(items.contains(nlink->inSocket->Socket.node) && !items.contains(nlink->outSocket->Socket.node))
            ins.append(nlink);
    return ins;
}

QList<NodeLink *> Shader_Space::getContainerOutLinks()
{
    QList<QGraphicsItem *> items = selectedItems();

    QList<NodeLink *> outs;

    foreach(NodeLink *nlink, links)
        if(!items.contains(nlink->inSocket->Socket.node) && items.contains(nlink->outSocket->Socket.node))
            outs.append(nlink);
    return outs;
}

QList<NodeLink *> Shader_Space::getContainerDataLinks()
{
    QList<QGraphicsItem *> items = selectedItems();

    QList<NodeLink *>linklist;
    foreach (NodeLink *nlink, links)
        if(items.contains(nlink->inSocket->Socket.node) && items.contains(nlink->outSocket->Socket.node))
            linklist.append(nlink);
    return linklist;
}

Node *Shader_Space::dropNode(QString filepath)
{
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);

    Node *node;
    stream>>&node;

    Node::loadIDMapper.clear();
    NSocket::loadIDMapper.clear();

    return node;
}

void Shader_Space::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if(itemAt(event->scenePos()))
    {
        QGraphicsScene::dragMoveEvent(event);
        return;
    }
    if(event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))
        event->acceptProposedAction();
}

void Shader_Space::shownodelib()
{
    if (!nodelib)
    {
        nodelib = new NodeLib(mousePos);
        nodelib->setScene(this);
    }
    else
        nodelib->container->setPos(mousePos);
}

void Shader_Space::enterEditNameMode()
{
    editNameMode = true;
}

void Shader_Space::leaveEditNameMode()
{
    editNameMode = false;
}

void Shader_Space::destroyContainer(QGraphicsItem *container)
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

void Shader_Space::createNode()
{
    nodeedit = new NewNodeEditor(mousePos);
    nodeedit->setScene(this);
};

QDataStream & operator<<(QDataStream &stream, Shader_Space *space)
{
    int nodecnt = space->nodecnt;
    stream<<(qint16)space->nodecnt;
    foreach(QGraphicsItem *item, space->items())
    {
        if(item->type() == Node::Type)
        {
            Node *node = (Node*)item;
            stream<<node;
        }
    }
    stream<<(qint16)space->links.size();
    foreach(NodeLink *nlink, space->links)
        stream<<nlink;
}

QDataStream & operator>>(QDataStream &stream, Shader_Space **space)
{
    Shader_Space *newspace = new Shader_Space;
    *space = newspace;
    qint16 nodecnt, lnkcnt;
    stream>>nodecnt;
    Node *node = 0;
    for(int i = 0; i<nodecnt; i++)
    {
        stream>>&node;
        newspace->addNode(node);
    }
    stream>>lnkcnt;
    NodeLink *nlink;
    for(int j = 0; j<lnkcnt; j++)
    {
        stream>>&nlink;
        nlink->inSocket->Socket.links.append(nlink);
        nlink->outSocket->Socket.links.append(nlink);
        nlink->inSocket->Socket.cntdSockets.append(&nlink->outSocket->Socket);
        nlink->outSocket->Socket.cntdSockets.append(&nlink->inSocket->Socket);
        newspace->links.append(nlink);
        newspace->addItem(nlink);
        newspace->connect(nlink, SIGNAL(removeLink(NodeLink*)), newspace, SLOT(removeLink(NodeLink*)));
    }
}
