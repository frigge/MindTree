#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void export3d(DataCache* cache)
{
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "TRANSFORMABLE";
    info.node_type = "EXPORT3D";
    info.cache_proc = export3d;
    return info;
}

void unload()
{
}
}
