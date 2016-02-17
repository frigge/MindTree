#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/cache_main.h"

using namespace MindTree;

void runIcosphere(DataCache* cache)
{
    auto obj = std::make_shared<GeoObject>();
    auto mesh = std::make_shared<MeshData>();
    auto verts = std::make_shared<VertexList>();
    auto polygon_color = std::vector<glm::vec4>();
    auto polys = std::make_shared<PolygonList>();

    verts->emplace_back(2, 1, 0);
    verts->emplace_back(0, 2, 1);
    verts->emplace_back(1, 0, 2);
    verts->emplace_back(-2, 1, 0); // 3
    verts->emplace_back(0, -2, 1);
    verts->emplace_back(1, 0, -2);
    verts->emplace_back(2, -1, 0);
    verts->emplace_back(0, 2, -1); // 7
    verts->emplace_back(-1, 0, 2);
    verts->emplace_back(-2, -1, 0);
    verts->emplace_back(0, -2, -1);
    verts->emplace_back(-1, 0, -2);

    polys->push_back({0, 1, 2});
    polygon_color.emplace_back(1, 0, 0, 1);
    polys->push_back({8, 2, 1});
    polygon_color.emplace_back(0, 1, 0, 1);
    polys->push_back({8, 1, 3});
    polygon_color.emplace_back(1, 1, 0, 1);
    polys->push_back({7, 3, 1});
    polygon_color.emplace_back(0, 0, 1, 1);
    polys->push_back({1, 0, 7});
    polygon_color.emplace_back(1, 0, 1, 1);
    polys->push_back({7, 0, 5});
    polygon_color.emplace_back(0, 1, 1, 1);
    polys->push_back({11, 7, 5});
    polygon_color.emplace_back(1, 1, 1, 1);

    mesh->setProperty("P", verts);
    mesh->setProperty("polygon", polys);
    mesh->setProperty("polygon_color", polygon_color);
    obj->setData(mesh);
    cache->pushData(obj);
}

extern "C" {
CacheProcessor* load()
{
    return new CacheProcessor("TRANSFORMABLE", "CREATEICOSPHERE", runIcosphere);
}

void unload()
{
}
}
