#ifndef NODE_DB_SBW4MSUR

#define NODE_DB_SBW4MSUR

#include "functional"
#include "string"
#include "vector"
#include "memory"

namespace MindTree
{
class DNode;
typedef std::shared_ptr<DNode> NodePtr;

class NodeType;
class AbstractNodeDecorator
{
public:
    AbstractNodeDecorator(std::string type="", std::string label="");
    virtual ~AbstractNodeDecorator();
    virtual NodePtr operator()(bool) = 0;
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
    BuildInDecorator(std::string type, std::string label, std::function<NodePtr(bool)> func);
    virtual ~BuildInDecorator();
    virtual NodePtr operator()(bool);

private:
    std::function<NodePtr(bool)> func;
};

class NodeDataBase
{
public:
    static void scanFolders();
    static void registerNodeType(AbstractNodeDecorator *factory);
    static std::vector<AbstractNodeDecorator*> getFactories();
    static NodePtr createNode(std::string name);
    static NodePtr createNodeByType(const NodeType &t);

private:
    static std::vector<AbstractNodeDecorator*> nodeFactories;

};
} /* MindTree */

#endif /* end of include guard: NODE_DB_SBW4MSUR */
