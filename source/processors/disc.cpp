#define GLM_SWIZZLE
#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "../plugins/datatypes/Object/dcel.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void createdisc(DataCache* cache)
{
    auto sides = cache->getData(0).getData<int>();

    double pi = std::acos(-1);
    auto mesh = std::make_shared<MeshData>();
    auto points = std::make_shared<VertexList>();
    auto polys = std::make_shared<PolygonList>();
    mesh->setProperty("P", points);
    mesh->setProperty("polygon", polys);

    dcel::Adapter dcel_data(mesh);

    //center vertex
    auto *center = dcel_data.newVertex();

    dcel::Vertex *lastVert = nullptr, *firstVert = nullptr;
    //create simple disk shape
    for(int i = 0; i < sides; ++i) {
        auto *vert = dcel_data.newVertex();
        dcel_data.connect(vert, center);

        if(i == 0) {
            lastVert = vert;
            firstVert = vert;
        }
        glm::vec3 p(sin(2 * pi * i/sides) * 0.5,
                    0,
                    cos(2 * pi * i/sides) * 0.5);
        vert->set("P", p);
        if(i == 0) continue;

        dcel_data.connect(vert, lastVert);
        dcel_data.fill({vert, lastVert, center});
        lastVert = vert;
    }

    dcel_data.connect(lastVert, firstVert);
    dcel_data.fill({lastVert, firstVert, center});
    dcel_data.updateMesh();

    cache->pushData(mesh);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "OBJECTDATA";
    info.node_type = "DISC";
    info.cache_proc = createdisc;
    return info;
}

void unload()
{
}
}
