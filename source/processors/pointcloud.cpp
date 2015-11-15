#include "data/debuglog.h"
#include "data/cache_main.h"

using namespace MindTree;

void run(DataCache* cache)
{
}

extern "C" {
CacheProcessor* load()
{
    dbout("new hot reloadable plugin loaded");
    return new CacheProcessor("TRANSFORMABLE", "CREATEPCPLANE", run);
}

void unload()
{
    dbout("hot reloadable plugin unloaded");
}
}
