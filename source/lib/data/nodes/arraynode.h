#ifndef MT_ARRAYNODE_H
#define MT_ARRAYNODE_H

#include "data/signal.h"
#include "data_node.h"

namespace MindTree {

class ArrayNode : public DNode
{
public:
    ArrayNode(bool raw);
    void setType(SocketType t);
    void addSocket(DSocket *socket);

private:
    SocketType type_;
    Signal::CallbackVector callbacks_;
};

}

#endif //MT_ARRAYNODE_H
