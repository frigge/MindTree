#ifndef NODE_DB_SBW4MSUR

#define NODE_DB_SBW4MSUR

#include "functional"
#include "string"
#include "vector"

namespace MindTree
{
class DNode;
class AbstractNodeFactory
{
public:
    AbstractNodeFactory(std::string type="", std::string label="");
    virtual ~AbstractNodeFactory();
    virtual DNode* operator()();
    void setLabel(std::string l);
    std::string getLabel();
    void setType(std::string t);
    std::string getType();

private:
    std::string type, label;
};

class BuildInFactory : public AbstractNodeFactory
{
public:
    BuildInFactory(std::string type, std::string label, std::function<DNode*()> func);
    virtual ~BuildInFactory();
    virtual DNode* operator()();

private:
    std::function<DNode*()> func;
};

class NodeDataBase
{
public:
    static void scanFolders();
    static void registerNodeType(AbstractNodeFactory *factory);
    static void unregisterNodeType();
    static std::vector<AbstractNodeFactory*> getFactories();
    static void regCB(std::function<void()> fun);
    static DNode* createNode(std::string name);

private:
    static std::vector<AbstractNodeFactory*> nodeFactories;
    static std::vector<std::function<void()>> registerCallbackStack;

};
} /* MindTree */

#endif /* end of include guard: NODE_DB_SBW4MSUR */
