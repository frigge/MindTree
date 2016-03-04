#ifndef MT_RELOADABLE_PLUGIN_H
#define MT_RELOADABLE_PLUGIN_H

#include "cache_main.h"

extern "C" {
struct CacheProcessorInfo {
    const char* socket_type;
    const char* node_type;
    void (*cache_proc)(MindTree::DataCache*);
};
}

#endif
