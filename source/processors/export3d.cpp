#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

using namespace MindTree;

class AssimpExporter
{
    std::vector<MeshData*> meshes_;
    std::vector<MaterialInstance*> materials_;
    std::string filename_;
    std::string format_;

public:
    AssimpExporter(const std::string &filename) :
        filename_(filename)
    {
    }
    
    void exportObject(const AbstractTransformablePtr &out)
    {
        Assimp::Exporter exporter;
        std::unordered_map<std::string, std::string> exportIDs;
        for(int i = 0; i < exporter.GetExportFormatCount(); ++i) {
            auto *desc = exporter.GetExportFormatDescription(i);
            exportIDs[desc->fileExtension] = desc->id;
        }

        aiScene scene;
        scene.mAnimations = nullptr;
        scene.mCameras = nullptr;
        scene.mFlags = 0;
        scene.mLights = nullptr;
        scene.mMeshes = nullptr;
        scene.mNumAnimations = 0;
        scene.mNumCameras = 0;
        scene.mNumLights = 0;
        scene.mNumMaterials = 0;
        scene.mNumMeshes = 0;
        scene.mNumTextures = 0;
        scene.mRootNode = writeObject(out);
        scene.mTextures = nullptr;
        writeMeshes(&scene);
        writeMaterials(&scene);

        auto format = filename_.substr(filename_.rfind(".") +1, std::string::npos);
        exporter.Export(&scene, exportIDs[format], filename_);
    }

    aiNode* writeObject(const AbstractTransformablePtr &trans)
    {
        auto *n = new aiNode();
        n->mTransformation = convert(trans->getTransformation());
        n->mMeshes = nullptr;

        if (trans->getType() == AbstractTransformable::GEO) {
            auto obj = std::static_pointer_cast<GeoObject>(trans);
            auto data = obj->getData();
            n->mMeshes = new uint[1];
            n->mMeshes[0] = meshes_.size();
            meshes_.push_back(std::static_pointer_cast<MeshData>(data).get());
            materials_.push_back(obj->getMaterial().get());
            n->mNumMeshes = 1;
        }

        auto size = trans->getChildren().size();
        n->mNumChildren = size;
        if(size > 0) {
            n->mChildren = new aiNode*[n->mNumChildren];
            uint i{0};
            for (const auto &child : trans->getChildren()) {
                n->mChildren[i] = writeObject(child);
                i++;
            }
        }

        return n;
    }

    void writeMeshes(aiScene *scene) const
    {
        uint i{0};
        scene->mMeshes = new aiMesh*[meshes_.size()];
        scene->mNumMeshes = meshes_.size();
        for(const auto &mesh : meshes_) {
            auto *aim = new aiMesh();
            scene->mMeshes[i] = aim;
            aim->mMaterialIndex = i;
            auto verts = mesh->getProperty("P").getData<VertexListPtr>();
            aim->mNumVertices = verts->size();
            aim->mVertices = new aiVector3D[verts->size()];
            aim->mNormals = new aiVector3D[verts->size()];
            auto normals = mesh->getProperty("N").getData<VertexListPtr>();
            for(int i = 0; i < verts->size(); ++i) {
                aim->mVertices[i] = convert((*verts)[i]);
                aim->mNormals[i] = convert((*normals)[i]);
            }

            auto polygons = mesh->getProperty("polygon").getData<PolygonListPtr>();

            aim->mFaces = new aiFace[polygons->size()];
            aim->mNumFaces = polygons->size();

            for (uint j = 0; j < polygons->size(); ++j) {
                aiFace f;
                f.mNumIndices = (*polygons)[j].size();
                f.mIndices = new uint[f.mNumIndices];
                for(uint k = 0; k < (*polygons)[j].size(); ++k) {
                    f.mIndices[k] = (*polygons)[j][k];
                }
                aim->mFaces[j] = f;
            }
            i++;
        }

    }

    void writeMaterials(aiScene *scene) const
    {
        scene->mMaterials = new aiMaterial*[materials_.size()];
        scene->mNumMaterials = materials_.size();
        uint i{0};
        for(const auto *m : materials_) {
            auto *mat = new aiMaterial();
            auto diff = convert(m->getProperty("diffuse_color").getData<glm::vec4>());
            float specint = m->getProperty("specular_intensity").getData<double>();
            float specrough = m->getProperty("specular_roughness").getData<double>();
            mat->AddProperty(&diff, 1, AI_MATKEY_COLOR_DIFFUSE);
            mat->AddProperty(&specint, 1, AI_MATKEY_SHININESS_STRENGTH);
            mat->AddProperty(&specrough, 1, AI_MATKEY_SHININESS);
            scene->mMaterials[i] = mat;
            ++i;
        }
    }

    aiVector3D convert(const glm::vec3 &vec) const
    {
        return aiVector3D(vec.x, vec.y, vec.z);
    }

    aiColor3D convert(const glm::vec4 &vec) const
    {
        return aiColor3D(vec.x, vec.y, vec.z);
    }

    aiMatrix4x4 convert(const glm::mat4 &mat) const
    {
        aiMatrix4x4 am;
        am.a1 = mat[0].x; am.a2 = mat[1].x; am.a3 = mat[2].x; am.a4 = mat[3].x;
        am.b1 = mat[0].y; am.b2 = mat[1].y; am.b3 = mat[2].y; am.b4 = mat[3].y;
        am.c1 = mat[0].z; am.c2 = mat[1].z; am.c3 = mat[2].z; am.c4 = mat[3].z;
        am.d1 = mat[0].w; am.d2 = mat[1].w; am.d3 = mat[2].w; am.d4 = mat[3].w;

        return am;
    }
};

void export3d(DataCache* cache)
{
    auto data = cache->getData(1).getData<AbstractTransformablePtr>();
    auto filename = cache->getData(0).getData<std::string>();

    AssimpExporter exporter(filename);
    exporter.exportObject(data);
}

extern "C" {
CacheProcessorInfo load()
{
    CacheProcessorInfo info;
    info.socket_type = "ACTION";
    info.node_type = "EXPORT3D";
    info.cache_proc = export3d;
    return info;
}

void unload()
{
}
}
