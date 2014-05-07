#ifndef PROJECT_H
#define PROJECT_H
#include "data/nodes/data_node.h"
#include "data/data_nodelink.h"
#include "data/dnspace.h"
#include "data/python/pyexposable.h"

namespace MindTree
{
    
class DNode;

class Project : public PyExposable
{
	Project(std::string filename="");

public:
	~Project();

    static Project* instance();
    static Project* create();
    static Project* load(std::string filename);

    void removeSelectedNodes(NodeList nodes);

    static std::vector<DNodeLink*> getOutLinks(NodeList nodes);
    static std::vector<DNodeLink*> getInLinks(NodeList nodes);
    static std::vector<DNodeLink*> getOutLinks(DNode *node);
    static std::vector<DNodeLink*> getInLinks(DNode *node);

    void save(); 
    void saveAs(); 
    void fromFile(std::string filename);

    std::string getFilename()const;
	void setFilename(std::string value);
	void setRootSpace(DNSpace* value);
	DNSpace* getRootSpace();

    void registerSpace(DNSpace *space);
    void unregisterSpace(DNSpace *space);

    std::string registerNode(DNode *node);
    std::string registerSocket(DSocket *socket);
    std::string registerSocketType(SocketType t);

    void unregisterItem(std::string idname);
    void unregisterItem(void *ptr);

    void* getItem(std::string idname);
    std::string getIDName(void *ptr);
    std::string registerItem(void* ptr, std::string name);

private:
    std::string filename;
    DNSpace *root_scene;
    static Project *_project;

    std::vector<DNSpace*>spaces;
    std::unordered_map<std::string, void*> idNames;
};
} /* MindTree */

#endif // PROJECT_H
