#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"
#include "../plugins/mtio/assimp.h"

using namespace MindTree;

void import(DataCache* cache)
{
    auto file = cache->getData(0).getData<std::string>();
    auto trans = cache->getData(1).getData<glm::mat4>();

    if(file.empty())
        return;

    AssimpImporter importer(file);

    auto ret = importer.import();
    ret->setTransformation(trans);
    cache->pushData(ret);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "TRANSFORMABLE";
    info.node_type = "IMPORT3D";
    info.cache_proc = import;
    return info;
}

void unload()
{
}
}
