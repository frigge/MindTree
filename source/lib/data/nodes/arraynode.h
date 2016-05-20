#ifndef MT_ARRAYNODE_H
#define MT_ARRAYNODE_H

#include "data/signal.h"
#include "data_node.h"

namespace MindTree {

class ArrayNode : public DNode
{
public:
    ArrayNode(bool raw=false);
};
}

#endif //MT_ARRAYNODE_H
