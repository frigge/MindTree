#include "create_list_node.h"

using namespace MindTree;

CreateListNode::CreateListNode(bool raw)
    : DNode("Create List")
{
    setType("CREATELIST");

    if(!raw) {
        auto *initValue = new DinSocket("Init Value", "VARIABLE", this);
        auto *cnt = new DinSocket("Count", "INTEGER", this);
        cnt->setProperty(1);

        auto *out = new DoutSocket("List", "LIST:VARIABLE", this);
        initValue->listenToLinked();
        out->setTypePropagationFunction([](SocketType t) {
            return "LIST:" + t.toStr();
        });
        out->listenToTypeChange(initValue);
    }
}
