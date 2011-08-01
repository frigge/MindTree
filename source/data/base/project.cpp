#include "project.h"

#include "QFileDialog"

#include "source/data/base/frg.h"
#include "source/graphics/base/vnspace.h"

Project::Project(QString filename)
    : filename(filename)
{
    FRG::CurrentProject = this;
    DNSpace *space;
    if(filename != "")
    {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QDataStream stream(&file);
        space = new DNSpace();
        stream>>&space;
        file.close();
        LoadNodeIDMapper::clear();
        LoadSocketIDMapper::clear();
    }
    else
    {
        space = new DNSpace;
    }
    setRootSpace(space);
    moveIntoRootSpace();
    root_scene->setName("Root");
}

Project::~Project()
{
    spaces.clear();
    delete root_scene;
    nodePositions.clear();
    FRG::SpaceDataInFocus = 0;
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
    stream<<getRootSpace();
    file.close();
}

void Project::saveAs()
{
    filename = QFileDialog::getSaveFileName(0);
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
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

QString Project::getFilename()
{
	return filename;
}

void Project::setFilename(QString value)
{
	filename = value;
}

DNSpace* Project::getRootSpace()
{
	return root_scene;
}

void Project::setRootSpace(DNSpace* value)
{
	root_scene = value;
}

void Project::moveIntoRootSpace()
{
    FRG::Space->moveIntoSpace(root_scene);
}

QList<DNodeLink*> Project::getInLinks(NodeList nodes)
{
	QList<DNodeLink*> ins;
    foreach(DNodeLink *dnlink, FRG::SpaceDataInFocus->getCachedLinks())
        if(nodes.contains(dnlink->in->getNode())
            &&!nodes.contains(dnlink->out->getNode()))
            ins.append(dnlink);
    return ins;
}

QList<DNodeLink*> Project::getInLinks(DNode *node)    
{
    NodeList nodes;
    nodes.append(node);
    return getInLinks(nodes);
}

QList<DNodeLink *> Project::getOutLinks(NodeList nodes)
{
    QList<DNodeLink *> outs;
    foreach(DNodeLink *dnlink, FRG::SpaceDataInFocus->getCachedLinks())
        if(nodes.contains(dnlink->out->getNode())
            &&!nodes.contains(dnlink->in->getNode()))
            outs.append(dnlink);
    return outs;
}

QList<DNodeLink*> Project::getOutLinks(DNode *node)    
{
    NodeList nodes;
    nodes.append(node);
    return getOutLinks(nodes);
}

QPointF Project::getNodePosition(DNode *node)
{
    if(!nodePositions.contains(node)) return QPointF(0, 0);
	return nodePositions.value(node);
}

void Project::setNodePosition(DNode *node, QPointF value)
{
    if(value == QPointF(0,0))
        nodePositions.remove(node);

	nodePositions[node] = value;
}

void Project::clearNodePosition(DNode *node)    
{
    nodePositions.remove(node);
}
