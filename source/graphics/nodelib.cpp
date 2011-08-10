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

#include	"source/graphics/base/vnspace.h"
#include "source/data/base/frg.h"

NodeLib::NodeLib(QWidget *parent):
    QTreeWidget(parent)
{
    setMinimumHeight(1);
    resize(200, 1);

    setHeaderHidden(true);
    setUpdatesEnabled(true);
    update();
    createContextMenu();
    setContextMenuPolicy(Qt::ActionsContextMenu);
    setAcceptDrops(true);
    setDragEnabled(true);

    setAlternatingRowColors(true);
    //setFrameStyle(QFrame::NoFrame);

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(renamed(QTreeWidgetItem*)));
}

NodeLib::~NodeLib()
{
//    nodelib_proxy->setParentItem(0);
//    delete nodelib_proxy;
//    delete container;
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
        QFileInfo fileinfo(item->text(1));
        QDir data(item->text(1));
        QString name = data.dirName();
        data.cdUp();
        if (fileinfo.isDir())
            data.rmdir(name);
        else
            data.remove(name);
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
    QTreeWidgetItem *hardnodes = new QTreeWidgetItem(QStringList("Build In"));
    addTopLevelItem(hardnodes);

    QTreeWidgetItem *inputs = new QTreeWidgetItem(QStringList("Inputs"));
    QTreeWidgetItem *surfin = new QTreeWidgetItem(QStringList() << "Surface Input" << "1");
    QTreeWidgetItem *dispin = new QTreeWidgetItem(QStringList() << "Displacement Input" << "2");
    QTreeWidgetItem *volin = new QTreeWidgetItem(QStringList() << "Volume Input" << "3");
    QTreeWidgetItem *lin = new QTreeWidgetItem(QStringList() << "Light Input" << "4");

    QTreeWidgetItem *outputs = new QTreeWidgetItem(QStringList("Outpus"));
    QTreeWidgetItem *surfout = new QTreeWidgetItem(QStringList() << "Surface Output" << "5");
    QTreeWidgetItem *dispout = new QTreeWidgetItem(QStringList() << "Displacement Output" << "6");
    QTreeWidgetItem *volout = new QTreeWidgetItem(QStringList() << "Volume Output" << "7");
    QTreeWidgetItem *lout = new QTreeWidgetItem(QStringList() << "Light Output" << "8");

    QTreeWidgetItem *math = new QTreeWidgetItem(QStringList("Math"));
    QTreeWidgetItem *madd = new QTreeWidgetItem(QStringList() << "Add" << "9");
    QTreeWidgetItem *msub = new QTreeWidgetItem(QStringList() << "Subtract" << "10");
    QTreeWidgetItem *mmult = new QTreeWidgetItem(QStringList() << "Multiply" << "11");
    QTreeWidgetItem *mdiv = new QTreeWidgetItem(QStringList() << "Divide" << "12");
    QTreeWidgetItem *mdot = new QTreeWidgetItem(QStringList() << "Dot Product" << "13");

    QTreeWidgetItem *cond = new QTreeWidgetItem(QStringList("Conditions"));
    QTreeWidgetItem *ifcont = new QTreeWidgetItem(QStringList() << "Condition Container" << "14");
    QTreeWidgetItem *cgr = new QTreeWidgetItem(QStringList() << "Greater Than"<< "15");
    QTreeWidgetItem *csm = new QTreeWidgetItem(QStringList() << "Smaller Than" << "16");
    QTreeWidgetItem *ceq = new QTreeWidgetItem(QStringList() << "Equal" << "17");
    QTreeWidgetItem *cnot = new QTreeWidgetItem(QStringList() << "Not" << "18");
    QTreeWidgetItem *cand = new QTreeWidgetItem(QStringList() << "And" << "19");
    QTreeWidgetItem *cor = new QTreeWidgetItem(QStringList() << "Or" << "20");

    QTreeWidgetItem *value = new QTreeWidgetItem(QStringList("Value"));
    QTreeWidgetItem *color = new QTreeWidgetItem(QStringList() << "Color" << "21");
    QTreeWidgetItem *string = new QTreeWidgetItem(QStringList() << "String" << "22");
    QTreeWidgetItem *float_ = new QTreeWidgetItem(QStringList() << "Float" << "23");

    QTreeWidgetItem *loop = new QTreeWidgetItem(QStringList("Loop"));
    QTreeWidgetItem *forloop = new QTreeWidgetItem(QStringList() << "For" << "24");
    QTreeWidgetItem *whileloop = new QTreeWidgetItem(QStringList() << "While" << "25");
    QTreeWidgetItem *illuminateloop = new QTreeWidgetItem(QStringList() << "Illuminate" << "26");
    QTreeWidgetItem *illuminanceloop = new QTreeWidgetItem(QStringList() << "Illuminance" << "27");
    QTreeWidgetItem *solarloop = new QTreeWidgetItem(QStringList() << "Solar" << "28");
    QTreeWidgetItem *gatherloop = new QTreeWidgetItem(QStringList() << "Gather" << "29");

    QTreeWidgetItem *preview = new QTreeWidgetItem(QStringList() << "Preview" << "30");

    QList<QTreeWidgetItem*> hardgrps, ins, outs, maths, conditionals, values, loops;
    hardgrps<<inputs<<outputs<<math<<cond<<value<<loop<<preview;
    ins<<surfin<<dispin<<volin<<lin;
    outs<<surfout<<dispout<<volout<<lout;
    maths<<madd<<msub<<mmult<<mdiv<<mdot;
    conditionals<<ifcont<<cgr<<csm<<ceq<<cnot<<cand<<cor;
    values<<color<<string<<float_;
    loops<<forloop<<whileloop<<illuminateloop<<illuminanceloop<<solarloop<<gatherloop;

    hardnodes->addChildren(hardgrps);
    inputs->addChildren(ins);
    outputs->addChildren(outs);
    math->addChildren(maths);
    cond->addChildren(conditionals);
    value->addChildren(values);
    loop->addChildren(loops);
    hardnodes->setExpanded(true);
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
    
    Qt::DropAction dropaction = drag->exec();
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
