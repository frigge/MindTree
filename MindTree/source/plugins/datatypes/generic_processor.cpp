#include "Object/object.h"
#include "generic_processor.h"
#include "data/properties.h"

using namespace MindTree;

BOOST_PYTHON_MODULE(generic_processor)
{
    auto values = [](DataCache *cache) { cache->pushData(cache->getData(0)); };

    PropertyConverter::registerConverter("FLOAT", 
                                         "INTEGER", 
                                         defaultPropertyConverter<double, int>);

    PropertyConverter::registerConverter("INTEGER", 
                                         "FLOAT", 
                                         defaultPropertyConverter<int, double>);

    DataCache::addProcessor("FLOAT", "FLOATVALUE", new CacheProcessor(values));
    DataCache::addProcessor("STRING", "STRINGVALUE", new CacheProcessor(values));
    DataCache::addProcessor("INTEGER", "INTVALUE", new CacheProcessor(values));
    DataCache::addProcessor("COLOR", "COLORVALUE", new CacheProcessor(values));
    DataCache::addProcessor("VECTOR3D", "VECTOR3DVALUE", new CacheProcessor(values));
    DataCache::addProcessor("VECTOR2D", "VECTOR2DVALUE", new CacheProcessor(values));
    DataCache::addProcessor("BOOLEAN", "BOOLVALUE", new CacheProcessor(values));

    DataCache::addProcessor("FLOAT", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<double>));

    DataCache::addProcessor("INTEGER", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<int>));

    DataCache::addProcessor("STRING", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<std::string>));

    DataCache::addProcessor("COLOR", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<glm::vec4>));

    DataCache::addProcessor("VECTOR3D", 
                            "ADD", 
                            new CacheProcessor(Cache::Generic::add<glm::vec3>));
}
