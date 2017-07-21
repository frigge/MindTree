#include <random>

#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"
#include "data/cache_main.h"

using namespace MindTree;

void run(DataCache* cache)
{
    float width = cache->getData(0).getData<double>();
    float height = cache->getData(1).getData<double>();
    int numPoints = cache->getData(2).getData<int>();

    auto points = std::make_shared<VertexList>();
    std::mt19937 engine;
    std::uniform_real_distribution<float> uniform_distribution;
    for(int i = 0; i< numPoints; ++i) {
        points->emplace_back(uniform_distribution(engine) * width,
                            0,
                            uniform_distribution(engine) * height);
    }

    MeshDataPtr mesh = std::make_shared<MeshData>();
    mesh->setProperty("P", points);
    GeoObjectPtr obj = std::make_shared<GeoObject>();
    obj->setData(mesh);
    cache->pushData(obj);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "TRANSFORMABLE";
    info.node_type = "CREATEPCPLANE";
    info.cache_proc = run;
    return info;
}

void unload()
{
    dbout("hot reloadable plugin unloaded");
}
}
