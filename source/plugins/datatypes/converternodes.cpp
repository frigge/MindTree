#define GLM_FORCE_SWIZZLE
#include "glm/glm.hpp"
#include "data/cache_main.h"
#include "converternodes.h"

using namespace MindTree;

void registerConverteNodeOperators()
{
    auto floatToColor = [](DataCache *cache)
    {
        double r = cache->getData(0).getData<double>();
        double g = cache->getData(1).getData<double>();
        double b = cache->getData(2).getData<double>();
        double a = cache->getData(3).getData<double>();

        cache->pushData(glm::vec4(r, g, b, a));
    };

    auto floatToVector3D = [](DataCache *cache)
    {
        double x = cache->getData(0).getData<double>();
        double y = cache->getData(1).getData<double>();
        double z = cache->getData(2).getData<double>();

        cache->pushData(glm::vec3(x, y, z));
    };

    auto floatToVector2D = [](DataCache *cache)
    {
        double x = cache->getData(0).getData<double>();
        double y = cache->getData(1).getData<double>();

        cache->pushData(glm::vec2(x, y));
    };

    auto vector3DToFloat = [](DataCache *cache)
    {
        glm::vec3 vec = cache->getData(0).getData<glm::vec3>();
        cache->pushData(vec.x);
        cache->pushData(vec.y);
        cache->pushData(vec.z);
    };

    auto vector2DToFloat = [](DataCache *cache)
    {
        glm::vec2 vec = cache->getData(0).getData<glm::vec2>();
        cache->pushData(vec.x);
        cache->pushData(vec.y);
    };

    auto colorToFloat = [](DataCache *cache)
    {
        glm::vec4 vec = cache->getData(0).getData<glm::vec4>();
        cache->pushData(vec.r);
        cache->pushData(vec.g);
        cache->pushData(vec.b);
        cache->pushData(vec.a);
    };

    DataCache::addProcessor(new CacheProcessor("COLOR", "FLOATTOCOLOR", floatToColor));
    DataCache::addProcessor(new CacheProcessor("VECTOR3D", "FLOATTOVECTOR3D", floatToVector3D));
    DataCache::addProcessor(new CacheProcessor("VECTOR2D", "FLOATTOVECTOR2D", floatToVector2D));
    DataCache::addProcessor(new CacheProcessor("FLOAT", "COLORTOFLOAT", colorToFloat));
    DataCache::addProcessor(new CacheProcessor("FLOAT", "VECTOR3DTOFLOAT", vector3DToFloat));
    DataCache::addProcessor(new CacheProcessor("FLOAT", "VECTOR2DTOFLOAT", vector2DToFloat));
}
