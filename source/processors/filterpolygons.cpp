#define GLM_SWIZZLE
#include <unordered_map>

#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void filter(DataCache* cache)
{
    auto input = cache->getData(0).getData<MeshDataPtr>();
    auto name = cache->getData(1).getData<std::string>();
    auto upper_limit = cache->getData(2).getData<double>();
    auto lower_limit = cache->getData(3).getData<double>();

    if(input
       && !input->hasProperty("P")
       && !input->hasProperty("polygon")
       &&!input->hasProperty(name))
        return;

    auto input_points = input->getProperty("P").getData<std::shared_ptr<VertexList>>();
    auto input_polys = input->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
    auto prop = input->getProperty(name).getData<std::vector<double>>();
    if(prop.size() != input_polys->size()) {
        cache->pushData(input);
        return;
    }

    auto points = std::make_shared<VertexList>();
    auto polygons = std::make_shared<PolygonList>();

    std::unordered_map<uint, uint> vertex_mapping;

    for(uint i = 0; i < input_polys->size(); ++i) {
        auto value = prop[i];
        if( value > lower_limit && value < upper_limit) {
            Polygon poly;
            const auto &old_poly = (*input_polys)[i];
            for(int j = 0; j < old_poly.size(); ++j) {
                if(vertex_mapping.find(old_poly[j]) == vertex_mapping.end()) {
                    vertex_mapping[old_poly[j]] = points->size();
                    points->push_back((*input_points)[old_poly[j]]);
                }
                poly.push_back(vertex_mapping[old_poly[j]]);
            }
            polygons->push_back(poly);
        }
    }

    auto mesh = std::make_shared<MeshData>();
    mesh->setProperty("P", points);
    mesh->setProperty("polygon", polygons);

    cache->pushData(mesh);
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
