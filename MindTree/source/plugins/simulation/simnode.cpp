#include "simnode.h"

using namespace MindTree;
SimulationNode::SimulationNode(bool raw)
    : ContainerNode("Simulation", raw)
{
    setNodeType("SIMULATION");

    if(!raw) {
        new DinSocket("step", "INTEGER", this);
    }
}

SimulationNode::~SimulationNode()
{
}
