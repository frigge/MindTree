#include "project.h"

#include "QFileDialog"

#include "source/graphics/base/vnspace.h"

Project::Project()
{
	setRootSpace(new DNSpace);
    root_scene->setSpaceVis(new VNSpace);
    root_scene->setName("Root");
	setCurrentSpace(root_scene);
	setFilename("");	
}

Project::~Project()
{
	foreach(DNode* node, nodes)
		delete node;
	nodes.clear();
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

QList<DNode*> Project::getNodes()
{
    return nodes;
}

void Project::addNode(DNode *node)
{
    nodes.append(node);
}

void Project::deleteNode(DNode *node)
{
    nodes.removeAll(node);
    delete node;
}

void Project::removeNode(DNode *node)
{
    nodes.removeAll(node);
}

void Project::removeSelectedNodes(QList<DNode *> nodes)
{
    foreach(DNode *node, nodes)
        removeNode(node);
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

DNSpace* Project::getCurrentSpace()
{
	return currentSpace;
}

void Project::setCurrentSpace(DNSpace* value)
{
	currentSpace = value;
}

