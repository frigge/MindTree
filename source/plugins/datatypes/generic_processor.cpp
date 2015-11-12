#include "Object/object.h"
#include "data/properties.h"
#include "converternodes.h"
#include "cmath"
#include "generic_processor.h"

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

    IO::Input::registerReader<double>();
    IO::Input::registerReader<std::string>();
    IO::Input::registerReader<int>();
    IO::Input::registerReader<bool>();
    IO::Input::registerReader<glm::vec3>();
    IO::Input::registerReader<glm::vec2>();
    IO::Input::registerReader<glm::ivec2>();
    IO::Input::registerReader<glm::vec4>();

    DataCache::addProcessor(new CacheProcessor("FLOAT", "FLOATVALUE", values));
    DataCache::addProcessor(new CacheProcessor("STRING", "STRINGVALUE", values));
    DataCache::addProcessor(new CacheProcessor("INTEGER", "INTVALUE", values));
    DataCache::addProcessor(new CacheProcessor("COLOR", "COLORVALUE", values));
    DataCache::addProcessor(new CacheProcessor("VECTOR3D", "VECTOR3DVALUE", values));
    DataCache::addProcessor(new CacheProcessor("VECTOR2D", "VECTOR2DVALUE", values));
    DataCache::addProcessor(new CacheProcessor("BOOLEAN", "BOOLVALUE", values));

    DataCache::addProcessor(new CacheProcessor("FLOAT",
                                               "ADD",
                                               Cache::Generic::add<double>));

    DataCache::addProcessor(new CacheProcessor("INTEGER",
                                               "ADD",
                                               Cache::Generic::add<int>));

    DataCache::addProcessor(new CacheProcessor("STRING",
                                               "ADD",
                                               Cache::Generic::add<std::string>));

    DataCache::addProcessor(new CacheProcessor("COLOR",
                                               "ADD",
                                               Cache::Generic::add<glm::vec4>));

    DataCache::addProcessor(new CacheProcessor("VECTOR3D",
                                               "ADD",
                                               Cache::Generic::add<glm::vec3>));

    DataCache::addProcessor(new CacheProcessor("FLOAT",
                                               "MULTIPLY",
                                               Cache::Generic::multiply<double>));

    DataCache::addProcessor(new CacheProcessor("INTEGER",
                                               "MULTIPLY",
                                               Cache::Generic::multiply<int>));

    auto sinfunc = [] (DataCache *cache) {
        auto value = cache->getData(0).getData<double>();

        cache->pushData(std::sin(value * 3.14159265359 / 180));
    };

    DataCache::addProcessor(new CacheProcessor("FLOAT", "SIN", sinfunc));

    registerConverteNodeOperators();
}
