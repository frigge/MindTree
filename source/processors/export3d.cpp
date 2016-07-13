#include "data/debuglog.h"
#include "../plugins/datatypes/Object/object.h"
#include "data/reloadable_plugin.h"

#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

using namespace MindTree;

class AssimpExporter
{
    std::vector<MeshData*> meshes_;
    std::vector<Material*> materials_;

public:
    AssimpExporter(const std::string &filename, const std::string &format) :
        filename_(filename), format_(format)
    {
    }
    
    void export(cosnt AbstractTransformablePtr &out)
    {
        Assimp::Exporter exporter;

        exporter.mScene = new aiScene();
        writeObject(out);
        writeMeshes(exporter.mScene);
        writeMaterials(exporter.mScene);
    }

    aiNode* writeObject(const AbstractTransformable &trans)
    {
        auto *n = new aiNode();
        n->mTransformation = convert(trans->getTransformation());

        if (trans->getType() == AbstractTransformable::GEOOBJECT) {
            auto data = std::static_pointer_cast<GeoObject>(trans)->getData();
            n->mMesh = meshes_.size();
            meshes_->push_back(std::static_pointer_cast<MeshData>(data));
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

    void writeMeshes() const
    {
        auto *mesh = new aiMesh();
        for(const auto &mesh : meshes_) {
            auto verts = mesh->getProperty("P").getData<VertexListPtr>();
            mesh->mNumVertices = verts->size();
            mesh->mVertices = new aiVector3D[verts->size()];
            mesh->mNormals = new aiVector3D[verts->size()];
            auto normals = mesh->getProperty("N").getData<VertexListPtr>();
            for(int i = 0; i < verts->size(); ++i) {
                mesh->mVertices[i] = convert((*verts)[i]);
                mesh->mNormals[i] = convert((*normals)[i]);
            }

            auto polygons = mesh->getProperty("polygon").getData<PolygonListPtr>();
        }
    }

    void writeFace() const
    {
    }

    void writeMaterials() const
    {
    }

    aiVector3D convert(const glm::vec3 &vec) const
    {
        return aiVector3D(vec.x, vec.y, vec.z);
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
    auto data = cache.getData(0).getData<AbstractTransformablePtr>();
    auto filename = cache.getData(1).getData<std::string>();
    auto format = cache.getData(2).getData<std::string>();

    AssimpExporter exporter(filename, format);
    exporter.export(data);
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
