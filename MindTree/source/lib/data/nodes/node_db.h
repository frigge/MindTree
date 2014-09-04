#ifndef NODE_DB_SBW4MSUR

#define NODE_DB_SBW4MSUR

#include "functional"
#include "string"
#include "vector"

namespace MindTree
{
class DNode;
class NodeType;
class AbstractNodeDecorator
{
public:
    AbstractNodeDecorator(std::string type="", std::string label="");
    virtual ~AbstractNodeDecorator();
    virtual DNode* operator()(bool);
    void setLabel(std::string l);
    std::string getLabel();
    void setType(std::string t);
    std::string getType();

private:
    std::string type, label;
};

class BuildInDecorator : public AbstractNodeDecorator
{
public:
    BuildInDecorator(std::string type, std::string label, std::function<DNode*(bool)> func);
    virtual ~BuildInDecorator();
    virtual DNode* operator()(bool);

private:
    std::function<DNode*(bool)> func;
};

class NodeDataBase
{
public:
    static void scanFolders();
    static void registerNodeType(AbstractNodeDecorator *factory);
    static std::vector<AbstractNodeDecorator*> getFactories();
    static DNode* createNode(std::string name);
    static DNode* createNodeByType(const NodeType &t);

private:
    static std::vector<AbstractNodeDecorator*> nodeFactories;

};
} /* MindTree */

#endif /* end of include guard: NODE_DB_SBW4MSUR */
