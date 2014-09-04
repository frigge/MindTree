#include "node_db.h"
#include "data/nodes/data_node.h"

using namespace MindTree;

std::vector<AbstractNodeDecorator*> NodeDataBase::nodeFactories;

AbstractNodeDecorator::AbstractNodeDecorator(std::string type, std::string label)
    : type(type), label(label)
{
    NodeType::registerType(type);
}

AbstractNodeDecorator::~AbstractNodeDecorator()
{
}

void AbstractNodeDecorator::setLabel(std::string l)    
{
    label=l;
}

std::string AbstractNodeDecorator::getLabel()    
{
    return label;
}

void AbstractNodeDecorator::setType(std::string t)    
{
    type = t;
}

std::string AbstractNodeDecorator::getType()    
{
    return type;
}

DNode* AbstractNodeDecorator::operator()(bool)    
{
}

BuildInDecorator::BuildInDecorator(std::string type, std::string label, std::function<DNode*(bool)> func)
    : AbstractNodeDecorator(type, label), func(func)
{
}

BuildInDecorator::~BuildInDecorator()
{
}

DNode* BuildInDecorator::operator()(bool raw)    
{
    return func(raw);
}

void NodeDataBase::registerNodeType(AbstractNodeDecorator *factory)    
{
    nodeFactories.push_back(factory);
}

DNode* NodeDataBase::createNode(std::string name)    
{
    for(auto fac : nodeFactories)
        if(fac->getLabel() == name)
            return (*fac)(false);

    std::cout << "node label \"" << name << "\" not found" << std::endl;
    return nullptr;
}

DNode* NodeDataBase::createNodeByType(const NodeType &t)
{
    for(auto fac : nodeFactories)
        if(t == fac->getType())
            return (*fac)(true);

    std::cout << "node type \"" << t.toStr() << "\" not found" << std::endl;
    return nullptr;
}

std::vector<AbstractNodeDecorator*> NodeDataBase::getFactories()    
{
    return nodeFactories;    
}
