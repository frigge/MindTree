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
    for(size_t i=0; i< cache->getNode()->getInSockets().size(); i++){
        data += cache->getData(i).getData<T>();
    }

    cache->pushData(data);
}

template <typename T>
void subtract(MindTree::DataCache *cache)
{
}

} // Generic
} // Cache
} // MindTree

#endif
