#ifndef GENERIC_PROCESSOR_H
#define GENERIC_PROCESSOR_H

#include "data/cache_main.h"

namespace MindTree
{
namespace Cache
{
namespace Generic
{

template <typename T>
void add(MindTree::DataCache * cache)
{
    T data;
    std::cout << "caching " << cache->getNode()->getNodeName() << std::endl;
    for(const auto *socket : cache->getNode()->getInSockets())
    {
        cache->cache(socket);
        std::cout << "current value is: " << cache->data.getData<T>() << std::endl;
        data += cache->data.getData<T>();
    }

    cache->data.setData<T>(data);
}

template <typename T>
void subtract(MindTree::DataCache *cache)
{
}

} // Generic
} // Cache
} // MindTree

#endif
