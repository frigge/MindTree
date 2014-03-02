#include "generic_processor.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(generic_processor)
{
    DataCache::addProcessor("FLOAT", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<double>));

    DataCache::addProcessor("INTEGER", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<int>));

    DataCache::addProcessor("STRING", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<std::string>));
}
