#ifndef GENERIC_PROCESSOR_H
#define GENERIC_PROCESSOR_H

#include "data/cache_main.h"

void initDefaultProperties();
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
    for(size_t i=0; i< cache->getNode()->getInSockets().size(); i++){
        std::cout << "current value is: " << cache->getData(i).getData<T>() << std::endl;
        data += cache->getData(i).getData<T>();
    }

    std::cout << "in combination it is: " << data << std::endl;
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
