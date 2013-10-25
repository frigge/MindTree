#include "project.h"

#include "QFileDialog"

#include "data/signal.h"
#include "data/frg.h"
//#include "source/graphics/base/vnspace.h"

using namespace MindTree;

Project* Project::project=0;

Project::Project(QString filename)
    : filename(filename)
{
    FRG::CurrentProject = this;
    DNSpace *space = 0;
    if(filename != "")
    {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QDataStream stream(&file);

        FRG_PROJECT_HEADER_CHECK
        {
            stream>>&space;
            file.close();
            LoadNodeIDMapper::clear();
            LoadSocketIDMapper::remap();
        }
        else
            space = new DNSpace;
    }
    else
    {
        space = new DNSpace;
    }
    setRootSpace(space);
    root_scene->setName("Root");
}

Project::~Project()
{
    spaces.clear();
    delete root_scene;
    nodePositions.clear();
    //FRG::SpaceDataInFocus = 0;
}

Project* Project::create()    
{
    if(project) delete project;
    project = new Project();
    return project;
}

Project* Project::load(std::string filename)    
{
    if(project) delete project;
    project = new Project(filename.c_str());
    return project;
}

Project* Project::instance()    
{
    return project;
}

//QString Project::registerViewer(ViewerBase *viewer)    
//{
//    return QString(); 
//}

QString Project::registerNode(DNode *node)    
{
    return QString(); 
}

QString Project::registerSocket(DSocket *socket)    
{
    return QString(); 
}

QString Project::registerSocketType(SocketType t)    
{
    return QString(); 
}

//QString Project::registerNodeType(NodeType t)    
//{
//    return QString(); 
//}

QString Project::registerItem(void* ptr, QString name)    
{
    int count=0;
    QString idname = name;
    while(IDNames.contains(idname)){
       count++;
       idname = name + QString::number(count);
    }
    IDNames.insert(idname, ptr);
    return idname;
}

QString Project::getIDName(void *ptr)    
{
    return IDNames.key(ptr);
}

void* Project::getItem(QString idname)    
{
    if(!IDNames.contains(idname))
        return 0;
    return IDNames.value(idname);
}

void Project::unregisterItem(void *ptr)    
{
    IDNames.remove(IDNames.key(ptr));
}

void Project::unregisterItem(QString idname)    
{
    IDNames.remove(idname);
}

void Project::save()
{
    if(getFilename() == "")
    {
        setFilename(QFileDialog::getSaveFileName(0));
    }
    QFile file(getFilename());
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream<<FRG_PROJECT_HEADER;
    stream<<getRootSpace();
    file.close();
}

void Project::saveAs()
{
    filename = QFileDialog::getSaveFileName(0);
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream<<FRG_PROJECT_HEADER;
    stream<<getRootSpace();
    file.close();
}

void Project::registerSpace(DNSpace *space)    
{
    spaces.append(space);
}

void Project::unregisterSpace(DNSpace *space)    
{
    spaces.removeAll(space);
}

QString Project::getFilename()const
{
	return filename;
}

void Project::setFilename(QString value)
{
	filename = value;
    MT_CUSTOM_SIGNAL_EMITTER("filename_changed", value);
}

DNSpace* Project::getRootSpace()
{
	return root_scene;
}

void Project::setRootSpace(DNSpace* value)
{
	root_scene = value;
}

//void Project::moveIntoRootSpace()
//{
//    FRG::Space->moveIntoSpace(root_scene);
//}

QList<DNodeLink*> Project::getOutLinks(NodeList nodes)
{
	QList<DNodeLink*> outs;
    foreach(DNode *node, nodes)
        foreach(DoutSocket* socket, node->getOutSockets())
            foreach(DNodeLink dnlink, socket->getLinks())
                if(!nodes.contains(dnlink.in->getNode()))
                    outs.append(new DNodeLink(dnlink));
    return outs;
}

QList<DNodeLink*> Project::getInLinks(DNode *node)    
{
    NodeList nodes;
    nodes.append(node);
    return getInLinks(nodes);
}

QList<DNodeLink *> Project::getInLinks(NodeList nodes)
{
    QList<DNodeLink *> ins;
    foreach(DNode *node, nodes)
        foreach(DinSocket *socket, node->getInSockets())
            if(socket->getCntdSocket()
                &&!nodes.contains(socket->getCntdSocket()->getNode()))
            ins.append(new DNodeLink(socket, socket->getCntdSocket()));
    return ins;
}

QList<DNodeLink*> Project::getOutLinks(DNode *node)    
{
    NodeList nodes;
    nodes.append(node);
    return getOutLinks(nodes);
}

QPointF Project::getNodePosition(const DNode *node)
{
    if(!nodePositions.contains(node)) return QPointF(0, 0);
	return nodePositions.value(node);
}

void Project::setNodePosition(const DNode *node, QPointF value)
{
    if(value == QPointF(0,0))
        nodePositions.remove(node);

	nodePositions[node] = value;
}

void Project::clearNodePosition(const DNode *node)    
{
    nodePositions.remove(node);
}
