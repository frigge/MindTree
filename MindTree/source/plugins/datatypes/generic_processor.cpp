#include "generic_processor.h"

using namespace MindTree;

void initDefaultProperties()
{
    PropertyData<std::string>::registerType("STRING");
    PropertyData<int>::registerType("INTEGER");
    PropertyData<float>::registerType("FLOAT");
    PropertyData<double>::registerType("FLOAT");
    PropertyData<bool>::registerType("BOOLEAN");
    PropertyData<glm::vec3>::registerType("VECTOR3D");
    PropertyData<glm::vec4>::registerType("COLOR");
    PropertyData<glm::vec2>::registerType("VECTOR2D");
}

BOOST_PYTHON_MODULE(generic_processor)
{
    auto values = [](const DataCache *) { };

    initDefaultProperties();

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
}
