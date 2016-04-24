#ifndef MT_ARRAYNODE_H
#define MT_ARRAYNODE_H

#include "data_node.h"

class ArrayNode : public DNode
{
public:
private:
    SocketType type_;
    CallbackVector callbacks_;
}

#endif //MT_ARRAYNODE_H
