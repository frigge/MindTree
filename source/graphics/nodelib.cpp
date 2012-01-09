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

#include "nodelib.h"

#include "QDir"
#include "QCoreApplication"
#include "stdio.h"
#include "QStandardItemModel"
#include "QApplication"
#include "QMimeData"
#include "QMouseEvent"
#include "QDrag"
#include "QTreeWidgetItemIterator"

#include	"source/graphics/base/vnspace.h"
#include "source/data/base/frg.h"
#include "source/data/nodes/buildin_nodes.h"

NodeLibWidget::NodeLibWidget(QWidget *parent)
    : QWidget(parent)
{
    FRG::lib = new NodeLib(this);
    BuildIn::registerNodes();
    FRG::lib->update();
    QVBoxLayout *lay = new QVBoxLayout();
    lay->setMargin(0);
    lay->setSpacing(0);
    setLayout(lay);
    filteredit = new QLineEdit(this);
    lay->addWidget(filteredit);

    lay->addWidget(FRG::lib);

    connect(filteredit, SIGNAL(textChanged(QString)), this, SLOT(filter()));

}

void NodeLibWidget::filter()    
{
    FRG::lib->filter(filteredit->text());
}

NodeLib::NodeLib(QWidget *parent):
    QTreeWidget(parent), NodeID(0)
{
    buildIns = new QTreeWidgetItem(QStringList() << "Build In");
    setMinimumHeight(1);
    resize(200, 1);

    setHeaderHidden(true);
    setUpdatesEnabled(true);
    update();
    createContextMenu();
    setContextMenuPolicy(Qt::ActionsContextMenu);
    setAcceptDrops(true);
    setDragEnabled(true);

    //setAlternatingRowColors(true);
    //setFrameStyle(QFrame::NoFrame);

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(renamed(QTreeWidgetItem*)));
}

NodeLib::~NodeLib()
{
//    nodelib_proxy->setParentItem(0);
//    delete nodelib_proxy;
//    delete container;
}

void NodeLib::filter(QString txt)    
{
    QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::NoChildren);
    while(*it){
        if(!(*it)->text(0).contains(txt))        
            (*it)->setHidden(true);
        else
        {
            (*it)->setHidden(false);
            QTreeWidgetItem *parent = (*it)->parent();
            while(parent){
                if(txt == "" && parent->parent())
                    parent->setExpanded(false);
                else
                    parent->setExpanded(true);
                parent = parent->parent();
            }
        }
        it++;
    }
}

void NodeLib::createContextMenu()
{
    QAction *removeaction = new QAction("remove", this);
    QAction *addFolderAction = new QAction("add Folder", this);
    addAction(removeaction);
    addAction(addFolderAction);

    connect(removeaction, SIGNAL(triggered()), this, SLOT(remove()));
    connect(addFolderAction, SIGNAL(triggered()), this, SLOT(addFolder()));
}

void NodeLib::remove()
{
    QList<QTreeWidgetItem *> items = selectedItems();
    foreach(QTreeWidgetItem *item, items)
    {
        FRG::Utils::remove(item->text(1));
        update();
    }
}

void NodeLib::addFolder()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QDir dir("nodes");
    dir.mkdir("New Folder");
    update();
}

void NodeLib::renamed(QTreeWidgetItem *item)
{
    QDir dir(item->text(1));
    QFileInfo fileinfo(item->text(1));
    QString newfilename = fileinfo.absoluteFilePath().replace(fileinfo.baseName(), item->text(0));
    dir.rename(fileinfo.absoluteFilePath(), newfilename);
    update();
}

void NodeLib::addBuildInNodes()
{
    addTopLevelItem(buildIns);
}

void NodeLib::addGroup(QString name, QString parent)    
{
    if(!NodeGroups.keys().contains(name)) {
        QTreeWidgetItem *newgroup = new QTreeWidgetItem(QStringList(name));
        NodeGroups.insert(name, newgroup);
        if(parent != ""){
            addGroup(parent);
            QTreeWidgetItem *parentItem = NodeGroups.value(parent);
            if(parentItem)
                parentItem->addChild(newgroup);
        }
        else{
            buildIns->addChild(newgroup);
        }
    }
}

void NodeLib::addNode(QString name, QString group, DNode*(*createFunc)())    
{
    addGroup(group);
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << name << QString::number(NodeID));

    NodeLibItem *nli = new NodeLibItem;
    nli->item = item;
    nli->dropFunc = createFunc;
    nodeLibItems.insert(NodeID, nli);
    QTreeWidgetItem *parentItem = NodeGroups.value(group);
    if(parentItem)
        parentItem->addChild(item);
    NodeID++;
}

NodeLibItem* NodeLib::getItem(int ID)    
{
    return nodeLibItems.value(ID); 
}

void NodeLib::clear()    
{
    takeTopLevelItem(0);
    QTreeWidget::clear();
}

void NodeLib::update()
{
    clear();
    /*adding hard coded nodes*/
    addBuildInNodes();

    /*adding Function and Container Nodes (Soft Code)*/
    QString appdirstr(QCoreApplication::applicationDirPath());
    QDir appdir(appdirstr);
    QDir::setCurrent(appdirstr);
    QDir libdir("nodes");

    if (!libdir.exists())
        appdir.mkdir("nodes");
    QTreeWidgetItem *libnodes = new QTreeWidgetItem(QStringList("Library"));
    addTopLevelItem(libnodes);
    libnodes->addChildren(addNodeItems(libdir));
    libnodes->setExpanded(true);

    QWidget::update();
}

QList<QTreeWidgetItem *> NodeLib::addNodeItems(QDir dir)
{
    QFileInfoList nodelist = dir.entryInfoList();
    QList<QTreeWidgetItem *> entrys;
    foreach(QFileInfo node, nodelist)
    {
        if ((node.completeSuffix() == "node"
             || node.completeSuffix() == "contnode")
            || (node.isDir() && !(node.baseName() == "")))
        {
            QList<QString> itemlist;
            itemlist << node.baseName() << node.filePath();
            QTreeWidgetItem *nodeitem = new QTreeWidgetItem(QStringList(itemlist));
            nodeitem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable |
                               Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
            if (node.isDir())
                nodeitem->addChildren(addNodeItems(QDir(node.filePath())));
            entrys.append(nodeitem);
        }
    }
    return entrys;
}

void NodeLib::mousePressEvent(QMouseEvent *event)    
{
    if(event->button() == Qt::LeftButton)
    {
        dragstartpos = event->pos();
    }
    QTreeWidget::mousePressEvent(event);
}

void NodeLib::mouseMoveEvent(QMouseEvent *event)    
{
//    if(!event->button() & Qt::LeftButton)
//        return;
    if(selectedItems().isEmpty()) return;
    if((event->pos() - dragstartpos).manhattanLength() < QApplication::startDragDistance())
        return;
            
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);

    QString datahint = selectedItems().first()->text(1);
    QFileInfo info(datahint);
    if (info.isFile())
    {
        stream<<datahint;
        mimeData->setData("FRGShaderAuthor/nodefile", itemData);
    }
    else if(info.isDir())
    {
        stream<<datahint;
        mimeData->setData("FRGShaderAuthor/nodedir", itemData);
    }
    else
    {
        stream<<datahint.toInt();
        mimeData->setData("FRGShaderAuthor/buildInNode", itemData);
    }
    drag->setMimeData(mimeData);
    
    drag->exec();
}

void NodeLib::dragEnterEvent(QDragEnterEvent *event)    
{
    if(event->mimeData()->hasFormat("FRGShaderAuthor/nodefile")
        ||event->mimeData()->hasFormat("FRGShaderAuthor/nodedir"))
        event->acceptProposedAction();
}

void NodeLib::dragMoveEvent(QDragMoveEvent *event)    
{
    if(!event->mimeData()->hasFormat("FRGShaderAuthor/nodefile")
        ||!event->mimeData()->hasFormat("FRGShaderAuthor/nodedir"))
        return;

    QTreeWidgetItem *item = itemAt(event->pos());
    if(item)
    {
        QFileInfo info(item->text(1));
        if(info.isDir())
            event->acceptProposedAction(); 
    }
}

void NodeLib::dropEvent(QDropEvent *event)    
{
    if (event->mimeData()->hasFormat("FRGShaderAuthor/nodefile")
        ||event->mimeData()->hasFormat("FRGShaderAuthor/nodedir"))
    {
        QTreeWidgetItem *parent = itemAt(event->pos());
        QString filename;
        QByteArray itemData;
        if (event->mimeData()->hasFormat("FRGShaderAuthor/nodefile"))
            itemData = event->mimeData()->data("FRGShaderAuthor/nodefile");
        else
            itemData = event->mimeData()->data("FRGShaderAuthor/nodedir");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream>>filename;
        QDir filepath(filename);
        QFileInfo fileinfo(filename);
        if(!fileinfo.exists())
            return;
        QDir destdir;
        if (parent)
        {
            destdir = QDir(parent->text(1));
            QFileInfo pinfo(parent->text(1));
            if(pinfo.isDir())
                FRG::Utils::moveDir(filepath, destdir);
        }
        else
        {
            destdir = QDir("nodes");
            FRG::Utils::moveDir(filepath, destdir);
        }
        update();
    }
    update();
}
