#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace MindTree;

class AssimpImporter
{
public:
    AssimpImporter(std::string filename) : filename_(filename) {}

    AbstractTransformablePtr import()
    {
        Assimp::Importer importer;

        auto flags = aiProcess_JoinIdenticalVertices
            | aiProcess_GenSmoothNormals
            | aiProcess_GenUVCoords
            | aiProcess_TransformUVCoords
            | aiProcess_FindInstances;

        auto *scene = importer.ReadFile(filename_, flags);

        meshes_.reserve(scene->mNumMeshes);
        for(int i = 0; i < scene->mNumMeshes; ++i) {
            parseMesh(scene->mMeshes[i]);
        }

        materials_.reserve(scene->mNumMaterials);
        for(int i = 0; i < scene->mNumMaterials; ++i) {
            parseMaterial(scene->mMaterials[i]);
        }

        return traverseScene(scene->mRootNode);
    }

private:
    glm::mat4 convert(const aiMatrix4x4 &ait) const
    {
        return glm::mat4(ait.a1, ait.a2, ait.a3, ait.a4,
                         ait.b1, ait.b2, ait.b3, ait.b4,
                         ait.c1, ait.c2, ait.c3, ait.c4,
                         ait.d1, ait.d2, ait.d3, ait.d4);
    }

    AbstractTransformablePtr traverseScene(const aiNode *n)
    {
        AbstractTransformablePtr ret;
        if(n->mNumMeshes == 1) {
            auto obj = std::make_shared<GeoObject>();
            auto m = meshes_[n->mMeshes[0]];
            
            if(m.material > 0){
                auto mat = std::make_shared<MaterialInstance>(materials_[m.material]);
                obj->setMaterial(mat);
            }
            obj->setData(m.mesh);
            ret = obj;
        }
        else {
            auto empty = std::make_shared<Empty>();
            ret = empty;
        }

        ret->setName(n->mName.C_Str());
        ret->setTransformation(convert(n->mTransformation));

        for(int i = 0; i < n->mNumChildren; ++i) {
            ret->addChild(traverseScene(n->mChildren[i]));
        }
        return ret;
    }

    void parseMesh(aiMesh *aim)
    {
        auto mesh = std::make_shared<MeshData>();
        auto verts = std::make_shared<VertexList>();
        auto n = std::make_shared<VertexList>();
        auto polygons = std::make_shared<PolygonList>();
        mesh->setProperty("P", verts);
        mesh->setProperty("N", n);
        mesh->setProperty("polygon", polygons);
        verts->reserve(aim->mNumVertices);

        for(int i = 0; i < aim->mNumVertices; ++i) {
            verts->push_back(glm::vec3(aim->mVertices[i].x,
                                   aim->mVertices[i].y,
                                   aim->mVertices[i].z));
            n->push_back(glm::vec3(aim->mNormals[i].x,
                                   aim->mNormals[i].y,
                                   aim->mNormals[i].z));
        }

        for(int i = 0; i < aim->mNumFaces; ++i) {
            Polygon p;
            for(int j = 0; j < aim->mFaces[i].mNumIndices; j++) {
                p.push_back(aim->mFaces[i].mIndices[j]);
            }
            polygons->push_back(p);
        }

        meshes_.push_back(MeshInfo{mesh, aim->mMaterialIndex});
    }

    void parseMaterial(aiMaterial *aim)
    {
        MaterialPtr mat = std::make_shared<Material>();
        materials_.push_back(mat);
    }

    struct MeshInfo {
        MeshDataPtr mesh;
        long material{-1};
    };

    std::string filename_;
    std::vector<MeshInfo> meshes_;
    std::vector<MaterialPtr> materials_;
};


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
