#define GLM_SWIZZLE
#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "../plugins/datatypes/Object/skeleton.h"
#include "data/reloadable_plugin.h"
#include "common.h"

using namespace MindTree;

std::shared_ptr<MeshData> meshJoint(JointPtr joint, MeshData *parent=nullptr)
{
    auto trans = joint->getTransformation();

    std::shared_ptr<MeshData> cylinder;
    //if no translation, skip, except for root
    if(trans[3] != glm::vec4(0, 0, 0, 1)
       || !joint->getParent()) {
        auto worldTrans = joint->getWorldTransformation();
        cylinder = createCylinder(3, false);
        auto verts = cylinder->getProperty("P").getData<std::shared_ptr<VertexList>>();
        for(auto &v : *verts) {
            v = (worldTrans * glm::vec4(v, 1)).xyz();
        }
        if (parent) {
            auto pverts = parent->getProperty("P").getData<std::shared_ptr<VertexList>>();
            auto offset = verts->size();
            pverts->insert(pverts->end(), verts->begin(), verts->end());

            auto ppolys = parent->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
            auto polys = cylinder->getProperty("polygon").getData<std::shared_ptr<PolygonList>>();
            for (auto &face : *polys) {
                for(auto &i : face)
                    i += offset;
            }
            ppolys->insert(ppolys->end(), polys->begin(), polys->end());
        }
    }

    for(auto j : joint->getChildren()) {
        if (j->getType() == AbstractTransformable::JOINT)
            meshJoint(std::static_pointer_cast<Joint>(j), parent);
    }

    return cylinder;
}

void meshing(DataCache* cache)
{
    auto joints = cache->getData(0).getData<JointPtr>();
    cache->pushData(meshJoint(joints)); 
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "OBJECTDATA";
    info.node_type = "MESHING";
    info.cache_proc = meshing;
    return info;
}

void unload()
{
}
}
