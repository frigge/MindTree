#ifndef MT_RELOADABLE_PLUGIN_H
#define MT_RELOADABLE_PLUGIN_H

#include "cache_main.h"

namespace MindTree {
namespace plugin {

struct CacheProcessorInfo {
    std::string socket_type;
    std::string node_type;
    void (*cache_proc)(DataCache*);
};

}
}
#endif
