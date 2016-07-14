#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void runIcosphere(DataCache* cache)
{
    auto mesh = std::make_shared<MeshData>();
    auto verts = std::make_shared<VertexList>();
    auto base_polys = PolygonList();
    auto polys = std::make_shared<PolygonList>();

    verts->push_back(glm::normalize(glm::vec3(2, 1, 0)));
    verts->push_back(glm::normalize(glm::vec3(0, 2, 1)));
    verts->push_back(glm::normalize(glm::vec3(1, 0, 2)));
    verts->push_back(glm::normalize(glm::vec3(-2, 1, 0)));
    verts->push_back(glm::normalize(glm::vec3(0, -2, 1)));
    verts->push_back(glm::normalize(glm::vec3(1, 0, -2)));
    verts->push_back(glm::normalize(glm::vec3(2, -1, 0)));
    verts->push_back(glm::normalize(glm::vec3(0, 2, -1)));
    verts->push_back(glm::normalize(glm::vec3(-1, 0, 2)));
    verts->push_back(glm::normalize(glm::vec3(-2, -1, 0)));
    verts->push_back(glm::normalize(glm::vec3(0, -2, -1)));
    verts->push_back(glm::normalize(glm::vec3(-1, 0, -2)));

    base_polys.push_back({0, 1, 2});
    base_polys.push_back({8, 2, 1});
    base_polys.push_back({8, 1, 3});
    base_polys.push_back({7, 3, 1});
    base_polys.push_back({1, 0, 7});
    base_polys.push_back({7, 0, 5});
    base_polys.push_back({11, 7, 5});
    base_polys.push_back({7, 11, 3});
    base_polys.push_back({0, 2, 6});
    base_polys.push_back({5, 0, 6});
    base_polys.push_back({2, 4, 6});
    base_polys.push_back({4, 10, 6});
    base_polys.push_back({4, 2, 8});
    base_polys.push_back({5, 6, 10});
    base_polys.push_back({5, 10, 11});
    base_polys.push_back({9, 3, 11});
    base_polys.push_back({8, 3, 9});
    base_polys.push_back({4, 8, 9});
    base_polys.push_back({4, 9, 10});
    base_polys.push_back({10, 9, 11});

    int iter = std::max(1, cache->getData(1).getData<int>());
    for (int i = 1; i < std::min(iter, 16); i++) {
        PolygonList new_polys;
        for (const auto &p : base_polys) {
            unsigned ofs = verts->size();

            //subdivide edges
            unsigned vi0 = p[0];
            unsigned vi1 = p[1];
            unsigned vi2 = p[2];
            glm::vec3 v0 = verts->at(vi0);
            glm::vec3 v1 = verts->at(vi1);
            glm::vec3 v2 = verts->at(vi2);
            glm::vec3 new_v0 = glm::normalize(v0 + (v1 - v0) * 0.5f);
            glm::vec3 new_v1 = glm::normalize(v0 + (v2 - v0) * 0.5f);
            glm::vec3 new_v2 = glm::normalize(v1 + (v2 - v1) * 0.5f);
            verts->push_back(new_v0);
            verts->push_back(new_v1);
            verts->push_back(new_v2);

            //create 4 new polygons
            new_polys.push_back({ofs, vi1, ofs + 2});
            new_polys.push_back({ofs + 2, vi2, ofs + 1});
            new_polys.push_back({ofs, ofs+1, vi0});
            new_polys.push_back({ofs, ofs + 2, ofs + 1});
        }
        base_polys = new_polys;
    }
    *polys = base_polys;

    mesh->setProperty("P", verts);
    mesh->setProperty("polygon", polys);
    mesh->computeVertexNormals();
    cache->pushData(mesh);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "OBJECTDATA";
    info.node_type = "CREATEICOSPHEREDATA";
    info.cache_proc = runIcosphere;
    return info;
}

void unload()
{
}
}
