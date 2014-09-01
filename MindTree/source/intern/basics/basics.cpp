#include "boost/python.hpp"

#include "data/cache_main.h"
#include "data/nodes/node_db.h"
#include "data/nodes/containernode.h"

#include "basics.h"

using namespace MindTree;
BOOST_PYTHON_MODULE(basics)
{
    auto *containerNodeDecorator = 
        new MindTree::BuildInDecorator("CONTAINER", 
                                     "General.Container", 
                                     [](bool raw)->DNode*{ return new ContainerNode("Container", raw); });

    NodeDataBase::registerNodeType(containerNodeDecorator);

    auto stepIn = [] (DataCache *cache) {
        const auto *cont = cache->getNode()->getDerivedConst<ContainerNode>();
        DataCache contCache;
        contCache.setNode(cont->getOutputs());
        int i = 0;
        for(auto *out : cont->getOutSockets()) {
            contCache.setType(out->getType());
            cache->pushData(contCache.getData(i));
            ++i;
        }
    };

    auto stepOut = [] (DataCache *cache) {
        const auto *node = cache->getNode()->getDerivedConst<SocketNode>();
        const auto *container = node->getContainer();
        DataCache contCache;
        contCache.setNode(container);
        int i = 0;
        for(auto *out : node->getOutSockets()) {
            contCache.setType(out->getType());
            cache->pushData(contCache.getData(i));
            ++i;
        }
    };

    DataCache::addGenericProcessor("CONTAINER", new CacheProcessor(stepIn));
    DataCache::addGenericProcessor("INSOCKETS", new CacheProcessor(stepOut));
}
