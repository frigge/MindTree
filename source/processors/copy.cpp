#define GLM_FORCE_SWIZZLE
#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

using namespace MindTree;

void copy(DataCache* cache)
{
    auto obj = cache->getData(0).getData<GeoObjectPtr>();
    auto pobj = cache->getData(1).getData<GeoObjectPtr>();
    auto use_normal = cache->getData(2).getData<bool>();
    auto upvector = cache->getData(3).getData<glm::vec3>();

    auto mesh = std::static_pointer_cast<MeshData>(pobj->getData());
    if(!mesh)
        return;

    if(!mesh->hasProperty("P"))
        return;
    
    auto points = mesh->getProperty("P").getData<std::shared_ptr<VertexList>>();
    std::shared_ptr<VertexList> normals;

    if(use_normal && mesh->hasProperty("N"))
        normals = mesh->getProperty("N").getData<std::shared_ptr<VertexList>>();

    auto empty = std::make_shared<Empty>();

    int i{0};
    for(const auto &p : *points) {
        auto copy = obj->clone();
        if(normals) {
            auto n = glm::normalize((*normals)[i]);
            auto x = glm::normalize(glm::cross(n, glm::normalize(upvector)));
            auto z = glm::normalize(glm::cross(x, n));

            glm::mat4 rot(glm::vec4(x, 0),
                          glm::vec4(n, 0),
                          glm::vec4(z, 0),
                          glm::vec4(0, 0, 0, 1));

            copy->setTransformation(rot * copy->getTransformation());
        }
        copy->setPosition(p);
        empty->addChild(copy);
        ++i;
    }

    cache->pushData(empty);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "TRANSFORMABLE";
    info.node_type = "COPY";
    info.cache_proc = copy;
    return info;
}

void unload()
{
}
}
