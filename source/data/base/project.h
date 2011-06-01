#ifndef PROJECT_H
#define PROJECT_H

#include "QList"
#include "source/data/nodes/data_node.h"
#include "source/graphics/nodelink.h"
#include "source/data/base/dnspace.h"

class DNode;

class Project
{
public:
	Project();
	~Project();
    QList<DNode*> getNodes();
    void addNode(DNode*);
    void deleteNode(DNode*);
    void removeNode(DNode *node);
    void removeSelectedNodes(QList<DNode*>nodes);

    static QList<NodeLinkData> getContainerOutLinks();
    static QList<NodeLinkData> getContainerInLinks();

    void setDNSpace(DNSpace *space); //copy to project

    void save(); //copy to project
    void saveAs(); //copy to project

	QString getFilename();
	void setFilename(QString value);
	void setRootSpace(DNSpace* value);
	DNSpace* getRootSpace();
	DNSpace* getCurrentSpace();
	void setCurrentSpace(DNSpace* value);

private:
    QString filename; //copy to project
    DNSpace *root_scene; //copy to project
    QList<DNode*> nodes;
    DNSpace *currentSpace;
};

#endif // PROJECT_H
