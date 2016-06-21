#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"
#include "common.h"

using namespace MindTree;

void cylinder(DataCache* cache)
{
    unsigned sides = std::max(3, cache->getData(0).getData<int>());
    bool cap = cache->getData(1).getData<bool>();
    cache->pushData(createCylinder(sides, cap));
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "OBJECTDATA";
    info.node_type = "CYLINDER";
    info.cache_proc = cylinder;
    return info;
}

void unload()
{
}
}
