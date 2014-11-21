#include "simnode.h"

using namespace MindTree;
SimulationNode::SimulationNode(bool raw)
    : LoopNode("Simulation", raw)
{
    setType("SIMULATION");

    if(!raw) {
        new DinSocket("step", "INTEGER", this);
    }
    getLoopedInputs()->setType("SIMULATEDINPUTS");
}

SimulationNode::~SimulationNode()
{
}

std::unordered_map<const LoopNode*, std::unique_ptr<SimulationContext>> SimulationContext::_simCache;

SimulationContext::SimulationContext(const LoopNode *node)
    : CacheContext(node)
{
}

SimulationContext::~SimulationContext()
{
}

SimulationContext* SimulationContext::getContext(const LoopNode *node)
{
    if(_simCache.find(node) == end(_simCache)) {
        auto ptr = std::unique_ptr<SimulationContext>(new SimulationContext(node));
        _simCache[node] = std::move(ptr);
    }
    return _simCache[node].get();
}
