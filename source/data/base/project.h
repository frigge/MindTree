#ifndef PROJECT_H
#define PROJECT_H

#include "QList"
#include "source/data/nodes/data_node.h"

class DNode;

class Project
{
public:
    Project();
    QList<DNode*> getNodes();
    void addNode(DNode*);
    void deleteNode(DNode*);

private:
    void removeNode(DNode *node);
    void removeSelectedNodes(QList<DNode*>nodes);
    QList<DNode*> nodes;
};

#endif // PROJECT_H
