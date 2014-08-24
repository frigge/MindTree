#include "boost/python.hpp"

#include "data/nodes/node_db.h"
#include "data/nodes/containernode.h"

#include "basics.h"

BOOST_PYTHON_MODULE(basics)
{
    auto *containerNodeDecorator = 
        new MindTree::BuildInDecorator("CONTAINER", 
                                     "General.Container", 
                                     [](bool raw)->MindTree::DNode*{ return new MindTree::ContainerNode("Container", raw); });

    MindTree::NodeDataBase::registerNodeType(containerNodeDecorator);

}
