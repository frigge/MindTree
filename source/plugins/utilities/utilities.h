#ifndef MT_UTILITIES_PLUGIN_H
#define MT_UTILITIES_PLUGIN_H

#include "data/nodes/data_node.h"

namespace MindTree {

class SwitchNode : public DNode
{
public:
    SwitchNode(bool raw=false);
};

}

#endif
