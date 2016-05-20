#include "data/signal.h"
#include "arraynode.h"

using namespace MindTree;

ArrayNode::ArrayNode(bool raw) :
    DNode("ArrayNode")
{
    setType("ARRAYNODE");

    if(raw) return;

    auto *out = new DoutSocket("Output", "LIST:VARIABLE", this);
    setDynamicSocketsNode(DSocket::IN);
    getVarSocket()->listenToLinked();
    out->setTypePropagationFunction([](SocketType t) {
            return "LIST:" + t.toStr();
        });

    out->listenToTypeChange(getVarSocket());
}
