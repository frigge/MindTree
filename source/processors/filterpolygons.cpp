#define GLM_SWIZZLE
#include <unordered_map>

#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void filter(DataCache* cache)
{
    auto input = cache->getData(0).getData<MeshDataPtr>();
    auto filter = cache->getData(1).getData<std::string>();
    auto upper_limit = cache->getData(2).getData<float>();
    auto lower_limit = cache->getData(3).getData<float>();

    if(input
       && !input->hasProperty("P")
       && !input->hasProperty("polygon")
       &&!input->hasProperty(filter))
        return;

    auto input_points = input->getProperty("P").getData<std::shared_ptr<VertexList>>();
    auto input_polys = input->getProperty("P").getData<std::shared_ptr<PolygonList>>();
    auto prop = input->getProperty(filter);

    auto points = std::make_shared<VertexList>();
    auto polygons = std::make_shared<PolygonList>();

    auto properties = input->getProperties();
    std::unordered_map<uint, uint> vertex_mapping;

    for(const auto &prop : properties) {
        bool matched = true;
        if((!matched) || (matched)) {
            for(uint i = 0; i < polygons->size(); ++i) {
            }
        }
    }
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "OBJECTDATA";
    info.node_type = "FILTERPOLYGON";
    info.cache_proc = filter;
    return info;
}

void unload()
{
}
}
