#include "project.h"

Project::Project()
{
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
