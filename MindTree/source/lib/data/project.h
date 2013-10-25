#ifndef PROJECT_H
#define PROJECT_H

#include "QList"
#include "QPointF"
#include "data/nodes/data_node.h"
#include "data/data_nodelink.h"
#include "data/dnspace.h"
#include "data/python/pyexposable.h"

namespace MindTree
{
    
class DNode;

class Project : public PyExposable
{
	Project(QString filename="");

public:
	~Project();

    static Project* instance();
    static Project* create();
    static Project* load(std::string filename);

    void removeSelectedNodes(QList<DNode*>nodes);

    static QList<DNodeLink*> getOutLinks(NodeList nodes);
    static QList<DNodeLink*> getInLinks(NodeList nodes);
    static QList<DNodeLink*> getOutLinks(DNode *node);
    static QList<DNodeLink*> getInLinks(DNode *node);

    void save(); 
    void saveAs(); 

	QString getFilename()const;
	void setFilename(QString value);
	void setRootSpace(DNSpace* value);
	DNSpace* getRootSpace();
    //void moveIntoRootSpace();
	void setNodePosition(const DNode *node, QPointF value);
	QPointF getNodePosition(const DNode *node);
    void clearNodePosition(const DNode *node);

    void registerSpace(DNSpace *space);
    void unregisterSpace(DNSpace *space);
    //QString registerViewer(ViewerBase *viewer);
    QString registerNode(DNode *node);
    QString registerSocket(DSocket *socket);
    QString registerSocketType(SocketType t);
    //QString registerNodeType(NodeType t);
    void unregisterItem(QString idname);
    void unregisterItem(void *ptr);
    void* getItem(QString idname);
    QString getIDName(void *ptr);
    QString registerItem(void* ptr, QString name);

private:
    QString filename;
    DNSpace *root_scene;
    static Project *project;
	QHash<const DNode*, QPointF> nodePositions;
    QList<DNSpace*>spaces;
    QHash<QString, void*> IDNames;
};
} /* MindTree */

#endif // PROJECT_H
