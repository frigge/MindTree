#ifndef MT_ASSIMP_H
#define MT_ASSIMP_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace MindTree {
class AssimpImporter
{
public:
    AssimpImporter(std::string filename) : filename_(filename) {}
	AbstractTransformablePtr import();

private:
	glm::mat4 convert(const aiMatrix4x4 &ait) const;

	AbstractTransformablePtr traverseScene(const aiNode *n);
	void parseMesh(aiMesh *aim);

    template<typename T, int size> void read(aiMaterial *mat, std::string n, int n1, int n2,T* ret)
    {
        for(int i = 0; i < mat->mNumProperties; ++i) {
            if(mat->mProperties[i]->mKey.data == n) {
                for (int j = 0; j < size; j++){
                    ret[j] = ((T*)mat->mProperties[i]->mData)[j];
                }
            }
        }
    }

    void parseMaterial(aiMaterial *aim);

    struct MeshInfo {
        MeshDataPtr mesh;
        long material{-1};
    };

    std::string filename_;
    std::vector<MeshInfo> meshes_;
    std::vector<MaterialPtr> materials_;
};
} // namespace MindTree

#endif // MT_ASSIMP_H
