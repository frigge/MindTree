#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void cylinder(DataCache* cache)
{
    auto mesh = std::make_shared<MeshData>();
    auto verts = std::make_shared<VertexList>();
    auto polys = std::make_shared<PolygonList>();

    int sides = std::max(3, cache->getData(0).getData<int>());
    bool cap = cache->getData(1).getData<bool>();

    mesh->setProperty("P", verts);
    mesh->setProperty("polygon", polys);

    double pi = acos(-1);

    //upper side
    for(int i = 0; i < sides; ++i) {
        verts->emplace_back(sin(2 * i * pi/sides), 1, cos(2 * i * pi/sides));
    }

    for(int i = 0; i < sides; ++i) {
        verts->emplace_back(sin(2 * i * pi/sides), 0, cos(2 * i * pi/sides));
    }

    if(cap) verts->emplace_back(0, 0, 0);
    if(cap) verts->emplace_back(0, 1, 0);

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
