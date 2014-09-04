#ifndef MT_SIMULATION_H
#define MT_SIMULATION_H

#include "data/nodes/containernode.h"

namespace MindTree {
class SimulationNode : public ContainerNode
{
public:
    SimulationNode(bool raw = false);
    virtual ~SimulationNode();
};

}

#endif
