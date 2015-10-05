#ifndef NODE_DB_SBW4MSUR

#define NODE_DB_SBW4MSUR

#include "functional"
#include "string"
#include "vector"
#include "unordered_map"
#include "memory"

namespace MindTree
{
class DNode;
class DataType;
typedef std::shared_ptr<DNode> NodePtr;

class NodeType;
class AbstractNodeDecorator
{
public:
    AbstractNodeDecorator(std::string type="", std::string label="");
    virtual NodePtr operator()(bool);
    void setLabel(std::string l);
    std::string getLabel();
    void setType(std::string t);
    std::string getType();

private:
    virtual NodePtr createNode(bool raw)=0;
    void createChildNodes(NodePtr node);

    std::string type, label;
};

class BuildInDecorator : public AbstractNodeDecorator
{
public:
    BuildInDecorator(std::string type, std::string label, std::function<NodePtr(bool)> func);
    virtual NodePtr createNode(bool);

private:
    std::function<NodePtr(bool)> func;
};

class NodeDataBase
{
public:
    static void scanFolders();
    static void registerNodeType(std::unique_ptr<AbstractNodeDecorator> &&factory);
    static std::vector<AbstractNodeDecorator*> getFactories();
    static NodePtr createNode(std::string name);
    static NodePtr createNodeByType(const NodeType &t);
    static std::vector<AbstractNodeDecorator*> getConverters(DataType t);

private:
    static std::vector<std::unique_ptr<AbstractNodeDecorator>> nodeFactories;
    static std::unordered_map<std::string, std::vector<AbstractNodeDecorator*>> s_converters;
};
} /* MindTree */

#endif /* end of include guard: NODE_DB_SBW4MSUR */
