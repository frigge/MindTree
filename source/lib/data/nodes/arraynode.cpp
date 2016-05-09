#include "data/signal.h"
#include "arraynode.h"

using namespace MindTree;

ArrayNode::ArrayNode(bool raw)
{
    if(raw) return;

    new DoutSocket("Output", "VARIABLE", this);
    new DinSocket("Input", "VARIABLE", this);
}

void ArrayNode::setType(SocketType t)
{
    if (type_ != "VARIABLE")
        return;
}

void ArrayNode::addSocket(DSocket *socket)
{
    DNode::addSocket(socket);
    if(socket->getDir() == DSocket::OUT) return;

    auto cb = Signal::getBoundHandler<DoutSocket*>(socket)
        .connect("linkChanged", [this, socket](DoutSocket *out) {
                const auto &insockets = this->getInSockets();
                if (!out)
                    this->removeSocket(socket);
                else if(out == socket->toIn()->getCntdSocket()
                        || !std::all_of(insockets.begin(),
                                        insockets.end(),
                                        [](const auto *in) {
                                return in->getCntdSocket();
                            }))
                    return;

                else
                    new DinSocket("Input", this->type_, this);
            });

    callbacks_.push_back(cb);
}
