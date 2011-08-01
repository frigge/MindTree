#ifndef PROJECT_H
#define PROJECT_H

#include "QList"
#include "QPointF"
#include "source/data/nodes/data_node.h"
#include "source/graphics/nodelink.h"
#include "source/data/base/dnspace.h"

class DNode;

class Project 
{
public:
	Project(QString filename="");
	~Project();
    void removeSelectedNodes(QList<DNode*>nodes);
    void registerSpace(DNSpace *space);
    void unregisterSpace(DNSpace *space);

    static QList<DNodeLink*> getOutLinks(NodeList nodes);
    static QList<DNodeLink*> getInLinks(NodeList nodes);
    static QList<DNodeLink*> getOutLinks(DNode *node);
    static QList<DNodeLink*> getInLinks(DNode *node);

    void save(); 
    void saveAs(); 

	QString getFilename();
	void setFilename(QString value);
	void setRootSpace(DNSpace* value);
	DNSpace* getRootSpace();
    void moveIntoRootSpace();
	void setNodePosition(DNode *node, QPointF value);
	QPointF getNodePosition(DNode *node);
    void clearNodePosition(DNode *node);

private:
    QString filename;
    DNSpace *root_scene;
	QHash<DNode*, QPointF> nodePositions;
    QList<DNSpace*>spaces;
};

#endif // PROJECT_H
