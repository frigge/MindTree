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
void add(MindTree::DataCache *cache)
{
    T data = cache->getData(0).getData<T>();
    for(size_t i=1; i<cache->getNode()->getInSockets().size(); i++){
        if(cache->getNode()->getInSockets().at(i)->getType() != "VARIABLE")
            data += cache->getData(i).getData<T>();
    }

    cache->pushData(data);
}

template <typename T>
void subtract(MindTree::DataCache *cache)
{
    T value1 = cache->getData(0).getData<T>();
    T value2 = cache->getData(1).getData<T>();

    cache->pushData(value1 - value2);
}

template <typename T>
void multiply(MindTree::DataCache *cache)
{
    T data = cache->getData(0).getData<T>();
    for(size_t i=1; i<cache->getNode()->getInSockets().size(); i++){
        if(cache->getNode()->getInSockets().at(i)->getType() != "VARIABLE")
            data *= cache->getData(i).getData<T>();
    }

    cache->pushData(data);
}

} // Generic
} // Cache
} // MindTree

#endif
