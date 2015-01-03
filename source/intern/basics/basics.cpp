#include "boost/python.hpp"

#include "data/cache_main.h"
#include "data/nodes/node_db.h"
#include "data/nodes/containernode.h"
#include "data/dnspace.h"
#include "create_list_node.h"

#include "basics.h"

using namespace MindTree;

void regContainer()
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

void regForLoop()
{
    auto *decorator = 
        new MindTree::BuildInDecorator("FOR",
                                     "General.For", 
                                     [](bool raw)->DNode*{ return new ForNode(raw); });

    NodeDataBase::registerNodeType(decorator);

    auto forloopproc = [](DataCache *cache) {
        const ForNode *node = cache->getNode()->getDerivedConst<ForNode>(); 

        //cache start, end and step values
        int startval = cache->getData(0).getData<int>();
        int endval = cache->getData(1).getData<int>();
        int stepval = cache->getData(2).getData<int>();

        LoopCache c(node);

        const auto *out = cache->getStart();
        
        auto loopoutnode = node->getContainerData()->getNodes()[2];
        auto outsockets = node->getOutSockets();
        const auto bout = begin(outsockets);
        const auto eout = end(outsockets);
        int outindex = std::distance(bout, std::find(bout, eout, out));
        MindTree::DataCache loopCache(&c);
        loopCache.setType(out->getType());
        loopCache.setNode(loopoutnode);
        for(int i = startval; i < endval; i += stepval) {
            c.setStep(i);
            c.addData(outindex, loopCache.getData(outindex));
        }
        cache->pushData(loopCache.getData(outindex), outindex);
    };

    auto loopinproc = [](DataCache *cache) {
        SocketNode *ls = cache->getStart()
            ->getNode()->getDerived<SocketNode>();
        auto *container = ls->getContainer();
        auto *stepSocket = container->getInSockets()[2];

        if(container->getSocketOnContainer(cache->getStart()) == stepSocket) {
            const auto *loopCache = static_cast<const LoopCache*>(cache->getContext());
            cache->pushData(loopCache->getStep());
        }
        else{
            DataCache c;
            c.setNode(container);
            c.setType(c.getStart()->getType());
            auto outsockets = cache->getNode()->getOutSockets();
            for(size_t i = 3; i < outsockets.size(); ++i) {
                auto *out = outsockets[i];
                c.setType(out->getType());
                cache->pushData(c.getData(i));
            }
        }
    };

    auto loopedproc = [](DataCache *cache) {
        auto *node = cache->getNode()->getDerivedConst<LoopSocketNode>();
        auto *loopCache = static_cast<LoopCache*>(cache->getContext());
        int i = 0;
        DataCache c;
        for (auto *out : node->getOutSockets()) {
            if(out->getType() == "VARIABLE") continue;
            Property prop = loopCache->getData(i);
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

    DataCache::addGenericProcessor("FOR", new CacheProcessor(forloopproc));
    DataCache::addGenericProcessor("LOOPINPUTS", new CacheProcessor(loopinproc));
    DataCache::addGenericProcessor("LOOPINSOCKETS", new CacheProcessor(loopedproc));
}

void regWhileLoop()
{
    auto *decorator = 
        new MindTree::BuildInDecorator("WHILE", 
                                     "General.While", 
                                     [](bool raw)->DNode*{ return new WhileNode(raw); });

    NodeDataBase::registerNodeType(decorator);

    auto whileproc = [](DataCache *cache) {

    };

    DataCache::addGenericProcessor("WHILE", new CacheProcessor(whileproc));
}

void regForeachLoop()
{
    auto *decorator = 
        new MindTree::BuildInDecorator("FOREACH", 
                                     "General.Foreach", 
                                     [](bool raw)->DNode*{ return new ForeachNode(raw); });

    NodeDataBase::registerNodeType(decorator);

    auto foreachproc = [](DataCache *cache) {
        const auto *fornode = cache->getNode()->getDerivedConst<ForeachNode>();
        LoopCache loopCache(fornode);

        //catch all vectors
        std::vector<Property> vectors;
        for (size_t i = 0; i < fornode->getInSockets().size(); ++i) {
            auto vec = cache->getData(i);
            if(!vec.isList()) continue;
            vectors.push_back(vec);
        }

        //actual foreach
        size_t size = vectors[0].size();
        DataCache c(&loopCache);
        for(size_t i = 0; i < size; ++i) {
            int j = 0;
            for(const auto &vec : vectors) {
                if(vec.size() != size) continue;
                auto prop = Property::getItem(vec, i);
                loopCache.addData(j, prop);
                auto t = prop.getType();
                c.setType(t);
                c.setNode(fornode->getOutputs());
                Property::setItem(vectors[j], i, c.getData(j));
                ++j;
            }

        }
        for(auto vec : vectors) {
            cache->pushData(vec);
        }
    };

    DataCache::addGenericProcessor("FOREACH", new CacheProcessor(foreachproc));
}

void regCreateList()
{
    auto createListProc = [] (DataCache *cache) {
        Property defaultProp = cache->getData(0);
        auto count = cache->getData(1).getData<int>();

        Property list = defaultProp.createList(count);

        cache->pushData(list);
    };

    DataCache::addGenericProcessor("CREATELIST", new CacheProcessor(createListProc));

    NodeDataBase::registerNodeType(new BuildInDecorator("CREATELIST", "General.Create List", [] (bool raw) {
        return new CreateListNode(raw);
    }));
}

BOOST_PYTHON_MODULE(basics)
{
    regContainer();
    regForLoop();
    regWhileLoop();
    regForeachLoop();
    regCreateList();
}
