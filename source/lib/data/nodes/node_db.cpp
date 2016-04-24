#include "node_db.h"
#include "data/nodes/data_node.h"

using namespace MindTree;

std::vector<std::unique_ptr<AbstractNodeDecorator>> NodeDataBase::nodeFactories;
std::unordered_map<std::string, std::vector<AbstractNodeDecorator*>> NodeDataBase::s_converters;
std::vector<std::string> NodeDataBase::s_nonConverters;

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

NodePtr AbstractNodeDecorator::operator()(bool raw)
{
    return createNode(raw);
}

std::string AbstractNodeDecorator::getType()
{
    return type;
}

BuildInDecorator::BuildInDecorator(std::string type, std::string label, std::function<NodePtr(bool)> func)
    : AbstractNodeDecorator(type, label), func(func)
{
}

NodePtr BuildInDecorator::createNode(bool raw)
{
    return func(raw);
}

void NodeDataBase::setNotConvertible(std::string type)
{
    s_nonConverters.push_back(type);
}

void NodeDataBase::registerNodeType(std::unique_ptr<AbstractNodeDecorator> &&factory)
{
    NodePtr prototype = (*factory)(false);
    auto outsockets = prototype->getOutSockets();
    auto insockets = prototype->getInSockets();
    if(outsockets.size() == 1) {
        auto p = [&outsockets](const DinSocket *socket) {
            return outsockets[0]->getType() != socket->getType();
        };

        if (std::all_of(begin(insockets), end(insockets), p)) {
            std::string type_string = outsockets[0]->getType().toStr();
            if(s_converters.find(type_string) == end(s_converters)) {
                s_converters[type_string] = std::vector<AbstractNodeDecorator*>();
            }
            if (std::none_of(begin(s_nonConverters),
                               end(s_nonConverters),
                             [&type_string](const std::string &s) { return s == type_string; }))
                s_converters[type_string].push_back(factory.get());
        }
    }

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

std::vector<AbstractNodeDecorator*> NodeDataBase::getConverters(DataType t)
{
    std::vector<AbstractNodeDecorator*> ret;
    if(s_converters.find(t.toStr()) == end(s_converters))
        return ret;

    const auto &converters = s_converters[t.toStr()];
    ret.insert(converters.begin(), converters.end(), ret.end());

    const auto &universalConverters = NodeDataBase::getConverters("VARIABLE");
    ret.insert(universalConverters.begin(), universalConverters.end(), ret.end());

    return ret;
}

std::vector<AbstractNodeDecorator*> NodeDataBase::getFactories()
{
    std::vector<AbstractNodeDecorator*> factories;
    for(const auto &fac : nodeFactories) {
        factories.push_back(fac.get());
    }
    return factories;
}
