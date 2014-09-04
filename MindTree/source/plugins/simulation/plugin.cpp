#include "boost/python.hpp"
#include "data/nodes/node_db.h"
#include "simnode.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(simulation) {
    NodeDataBase::registerNodeType(new BuildInDecorator("SIMULATION", "General.Simulation", [] (bool raw) {
                                                            return new SimulationNode(raw);
                                                        }));
}
