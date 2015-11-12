#include "boost/python.hpp"
#include "data/cache_main.h"
#include "data/nodes/node_db.h"
#include "data/dnspace.h"
#include "simnode.h"

using namespace MindTree;

void regSimulation()
{
    auto simProc = [](DataCache *cache) {
        const auto *simNode = cache->getNode()->getDerivedConst<SimulationNode>();
        auto *cacheCtx = SimulationContext::getContext(simNode);

        const auto *out = cache->getStart();

        auto loopoutnode = simNode->getContainerData()->getNodes()[2];
        auto outsockets = simNode->getOutSockets();
        const auto bout = begin(outsockets);
        const auto eout = end(outsockets);

        int outindex = std::distance(bout, std::find(bout, eout, out));
        MindTree::DataCache simCache(cacheCtx);
        simCache.setType(out->getType());
        simCache.setNode(loopoutnode.get());

        auto data = simCache.getData(outindex);
        cacheCtx->addData(outindex, data);

        cache->pushData(data, outindex);
    };

    auto simulatedProc = [] (DataCache *cache) {
        auto *node = cache->getNode()->getDerivedConst<LoopSocketNode>();
        auto *cacheCtx = static_cast<CacheContext*>(cache->getContext());
        int i = 0;
        DataCache c;
        for (auto *out : node->getOutSockets()) {
            if(out->getType() == "VARIABLE") continue;
            Property prop = cacheCtx->getData(i);
            if(prop) {
                cache->pushData(prop);
            } else {
                c.setType(out->getType());
                auto *loopNode = node->getContainer();
                c.setNode(loopNode);
                auto *inOnCont = loopNode->getSocketOnContainer(out)->toIn();
                auto ins = loopNode->getInSockets();
                int inIndex = std::distance(begin(ins),
                                            std::find(begin(ins),
                                                      end(ins),
                                                      inOnCont));
                auto data = c.getData(inIndex);

                cache->pushData(data);
            }
            ++i;
        }
    };

    DataCache::addGenericProcessor(new GenericCacheProcessor("SIMULATION", simProc));
    DataCache::addGenericProcessor(new GenericCacheProcessor("SIMULATEDINPUTS", simulatedProc));
}

BOOST_PYTHON_MODULE(simulation) {
    auto decorator = std::make_unique<BuildInDecorator>("SIMULATION",
                                           "General.Simulation",
                                           [] (bool raw) {
                                                return std::make_shared<SimulationNode>(raw);
    });
    NodeDataBase::registerNodeType(std::move(decorator));

    regSimulation();
}
