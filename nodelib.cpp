#include "QDir"
#include "QCoreApplication"
#include "stdio.h"
#include "QStandardItemModel"
#include "QApplication"

#include "nodelib.h"
#include "scenewidgetcontainer.h"

NodeLib::NodeLib(QPointF atPos, QWidget *parent):
    QTreeWidget(parent)
{
    setMinimumHeight(1);
    resize(200, 1);

    nodelib_proxy = new QGraphicsProxyWidget;
    nodelib_proxy->setWidget(this);
    container = new SceneWidgetContainer;
    container->setProxy(nodelib_proxy);
    container->setPos(atPos);

    setHeaderHidden(true);
    setUpdatesEnabled(true);
    update();
    createContextMenu();
    setContextMenuPolicy(Qt::ActionsContextMenu);
    setAcceptDrops(true);
    setDragEnabled(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setAlternatingRowColors(true);
    //setFrameStyle(QFrame::NoFrame);

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(renamed(QTreeWidgetItem*)));
    connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(resizeLib()));
    connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(resizeLib()));
}

NodeLib::~NodeLib()
{
//    nodelib_proxy->setParentItem(0);
//    delete nodelib_proxy;
//    delete container;
}

float NodeLib::getLib_height()
{
    QTreeWidgetItem *item, *first_item;
    first_item = topLevelItem(0);
    item = first_item;
    int count = 0;
    while(item)
    {
        ++count;
        item = itemBelow(item);
    }

    return count * visualItemRect(first_item).height();
}

void NodeLib::resizeLib()
{
    resize(200, getLib_height());
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

void NodeLib::setScene(QGraphicsScene *scene)
{
    space = scene;
    space->addItem(container);
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

    QList<QTreeWidgetItem*> hardgrps, ins, outs, maths, conditionals, values, loops;
    hardgrps<<inputs<<outputs<<math<<cond<<value<<loop;
    ins<<surfin<<dispin<<volin<<lin;
    outs<<surfout<<dispout<<volout<<lout;
    maths<<madd<<msub<<mmult<<mdiv<<mdot;
    conditionals<<ifcont<<cgr<<csm<<ceq<<cnot<<cand<<cor;
    values<<color<<string<<float_;
    loops<<forloop<<whileloop;

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
    resizeLib();

    QWidget::update();
}

QList<QTreeWidgetItem *> NodeLib::addNodeItems(QDir dir, QTreeWidgetItem *parent)
{
    QFileInfoList nodelist = dir.entryInfoList();
    QList<QTreeWidgetItem *> entrys;
    foreach(QFileInfo node, nodelist)
    {
        if ((node.completeSuffix() == "node"
             || node.completeSuffix() == "contnode")
            || node.isDir() && !(node.baseName() == ""))
        {
            QList<QString> itemlist;
            itemlist << node.baseName() << node.filePath();
            QTreeWidgetItem *nodeitem = new QTreeWidgetItem(QStringList(itemlist));
            nodeitem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable |
                               Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
            if (node.isDir())
                nodeitem->addChildren(addNodeItems(QDir(node.filePath()), nodeitem));
            entrys.append(nodeitem);
        }
    }
    return entrys;
}

bool NodeLib::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
    if (data->hasFormat("application/x-qabstractitemmodeldatalist") && selectedItems().first() != parent)
    {
        QDir filepath(selectedItems().first()->text(1));
        QFileInfo fileinfo(selectedItems().first()->text(1));
        if(!fileinfo.exists())
            return false;
        QDir destdir;
        if (parent)
            destdir = QDir(parent->text(1));
        else
            destdir = QDir("nodes");
        moveNode(filepath, destdir);
        update();
        return true;
    }
}

void NodeLib::moveNode(QDir source, QDir destination)
{
    QFileInfo srcinfo(source.path());
    QDir destdir(destination);
    if (srcinfo.isDir())
    {
        QString srcdirname = source.dirName();
        destdir.mkdir(srcdirname);
        destdir.cd(srcdirname);

        QDir sourceparent(source);
        sourceparent.cdUp();
        QString sourcedirname(source.dirName());
        if (!sourceparent.rmdir(sourcedirname))
        {
            QFileInfoList sourceinfolist = source.entryInfoList();
            foreach (QFileInfo dirinfo, sourceinfolist)
            {
                QString dirinfofilepath = dirinfo.filePath();
                if (dirinfo.baseName() != "")
                    moveNode(QDir(dirinfo.filePath()), destdir);
            }
            sourceparent.rmdir(sourcedirname);
        }
    } else
    {
        QFile srcfile(source.path());
        QString filename(srcinfo.fileName());
        QString destfilepath(destdir.filePath(filename));
        if (srcfile.copy(destfilepath))
            srcfile.remove();
    }
}
