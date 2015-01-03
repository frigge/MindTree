#ifndef MT_SIMULATION_H
#define MT_SIMULATION_H

#include "data/cache_main.h"
#include "data/nodes/containernode.h"

namespace MindTree {
class SimulationNode : public LoopNode
{
public:
    SimulationNode(bool raw = false);
    virtual ~SimulationNode();
};

class SimulationContext : public CacheContext
{
public:
    SimulationContext(const LoopNode *node);
    ~SimulationContext();

    static SimulationContext* getContext(const LoopNode *node);

private:
    static std::unordered_map<const LoopNode*, std::unique_ptr<SimulationContext>> _simCache;
};

}

#endif
