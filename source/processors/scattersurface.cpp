#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void scattersurface(DataCache* cache)
{
    auto obj = cache->getData(0).getData<AbstractTransformablePtr>();
    unsigned count = cache->getData(1).getData<int>();
    if(!obj)
        return;

    auto geo = std::static_pointer_cast<GeoObject>(obj);
    if(!geo)
        return;

    auto mesh = std::static_pointer_cast<MeshData>(geo->getData());

    if(!mesh)
        return;

    if(!mesh->hasProperty("polygon")) return;

    auto polys = mesh->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
    auto verts = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();

    float area{0};
    for (const auto &poly : *polys) {
        int first = poly[0];
        auto v0 = (*verts)[first];
        float a{0};
        for(int i = 1; i < poly.size() - 1; ++i) {
            auto v1 = (*verts)[poly[i]];
            auto v2 = (*verts)[poly[i + 1]];

            auto e1 = v1 - v0;
            auto e2 = v2 - v0;
            a += glm::length(glm::cross(e1, e2)) * 0.5;
        }
        area += a;
    }

    struct PolygonTriangle {
        uint polygon;
        uint triangle;
    };

    std::map<float, PolygonTriangle> polygon_map;
    float offset{0};
    uint pi{0};
    for (const auto &poly : *polys) {
        uint first = poly[0];

        auto v0 = (*verts)[first];
        for(uint i = 1; i < poly.size() - 1; ++i) {
            auto v1 = (*verts)[poly[i]];
            auto v2 = (*verts)[poly[i + 1]];

            auto e1 = v1 - v0;
            auto e2 = v2 - v0;
            float a = glm::length(glm::cross(e1, e2)) * 0.5;

            offset += a/area;
            polygon_map[offset] = PolygonTriangle{pi, i};
        }
        pi++;
    }

    
    auto points = std::make_shared<VertexList>();
    std::mt19937 g;
    std::uniform_real_distribution<float> dist;

    for(int i = 0; i < count; ++i) {
        float pos = dist(g);
        glm::vec2 uv(dist(g), dist(g));
        if(uv.x + uv.y > 1.f)
            uv = glm::vec2(1) - uv;

        auto tri = polygon_map.upper_bound(pos);
        const auto &poly = (*polys)[tri->second.polygon];
        uint j = tri->second.triangle;
        auto v0 = (*verts)[poly[0]];
        auto v1 = (*verts)[poly[j]];
        auto v2 = (*verts)[poly[j + 1]];

        auto e1 = v1 - v0;
        auto e2 = v2 - v0;

        points->push_back(v0 + e1 * uv.x + e2 * uv.y);
    }

    MeshDataPtr retmesh = std::make_shared<MeshData>();
    retmesh->setProperty("P", points);
    GeoObjectPtr retobj = std::make_shared<GeoObject>();
    retobj->setData(retmesh);
    cache->pushData(retobj);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "TRANSFORMABLE";
    info.node_type = "SCATTERSURFACE";
    info.cache_proc = scattersurface;
    return info;
}

void unload()
{
}
}
