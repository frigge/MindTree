#include "node_db.h"
#include "data/nodes/data_node.h"

using namespace MindTree;

std::vector<std::unique_ptr<AbstractNodeDecorator>> NodeDataBase::nodeFactories;

AbstractNodeDecorator::AbstractNodeDecorator(std::string type, std::string label)
    : type(type), label(label)
{
    NodeType::registerType(type);
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

BuildInDecorator::BuildInDecorator(std::string type, std::string label, std::function<NodePtr(bool)> func)
    : AbstractNodeDecorator(type, label), func(func)
{
}

NodePtr BuildInDecorator::operator()(bool raw)
{
    return func(raw);
}

void NodeDataBase::registerNodeType(std::unique_ptr<AbstractNodeDecorator> &&factory)
{
    nodeFactories.push_back(std::move(factory));
}

NodePtr NodeDataBase::createNode(std::string name)
{
    for(const auto &fac : nodeFactories)
        if(fac->getLabel() == name)
            return (*fac)(false);

    std::cout << "node label \"" << name << "\" not found" << std::endl;
    return nullptr;
}

NodePtr NodeDataBase::createNodeByType(const NodeType &t)
{
    for(const auto &fac : nodeFactories)
        if(t == fac->getType())
            return (*fac)(true);

    std::cout << "node type \"" << t.toStr() << "\" not found" << std::endl;
    return nullptr;
}

std::vector<AbstractNodeDecorator*> NodeDataBase::getFactories()
{
    std::vector<AbstractNodeDecorator*> factories;
    for(const auto &fac : nodeFactories) {
        factories.push_back(fac.get());
    }
    return factories;
}
