#include "../plugins/datatypes/Object/skeleton.h"
#include "data/reloadable_plugin.h"
#include "data/cache_main.h"

using namespace MindTree;

void run(DataCache *cache)
{
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "TRANSFORMABLE";
    info.node_type = "LSYSTEMNODE";
    info.cache_proc = run;
    return info;
}

void unload()
{
    dbout("hot reloadable plugin unloaded");
}
}
