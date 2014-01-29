#include "node_db.h"
#include "data/nodes/data_node.h"

using namespace MindTree;

std::vector<AbstractNodeFactory*> NodeDataBase::nodeFactories;
std::vector<std::function<void()>> NodeDataBase::registerCallbackStack;

AbstractNodeFactory::AbstractNodeFactory(std::string type, std::string label)
    : type(type), label(label)
{
    NodeType::registerType(type);
}

AbstractNodeFactory::~AbstractNodeFactory()
{
}

void AbstractNodeFactory::setLabel(std::string l)    
{
    label=l;
}

std::string AbstractNodeFactory::getLabel()    
{
    return label;
}

void AbstractNodeFactory::setType(std::string t)    
{
    type = t;
}

std::string AbstractNodeFactory::getType()    
{
    return type;
}

DNode* AbstractNodeFactory::operator()()    
{
}

BuildInFactory::BuildInFactory(std::string type, std::string label, std::function<DNode*()> func)
    : AbstractNodeFactory(type, label), func(func)
{
}

BuildInFactory::~BuildInFactory()
{
}

DNode* BuildInFactory::operator()()    
{
    return func();
}

void NodeDataBase::scanFolders()    
{
}

void NodeDataBase::regCB(std::function<void()> fun)    
{
    registerCallbackStack.push_back(fun);
}

void NodeDataBase::registerNodeType(AbstractNodeFactory *factory)    
{
    nodeFactories.push_back(factory);
    for(auto f : registerCallbackStack) f();
}

DNode* NodeDataBase::createNode(std::string name)    
{
    for(auto fac : nodeFactories)
        if(fac->getLabel() == name)
            return (*fac)();
    return 0;
}

void NodeDataBase::unregisterNodeType()    
{
}

std::vector<AbstractNodeFactory*> NodeDataBase::getFactories()    
{
    return nodeFactories;    
}
