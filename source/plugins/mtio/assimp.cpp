#include "../plugins/datatypes/Object/object.h"

#include "assimp.h"

using namespace MindTree;

AbstractTransformablePtr AssimpImporter::import()
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
glm::mat4 AssimpImporter::convert(const aiMatrix4x4 &ait) const
{
	return glm::mat4(ait.a1, ait.a2, ait.a3, ait.a4,
						ait.b1, ait.b2, ait.b3, ait.b4,
						ait.c1, ait.c2, ait.c3, ait.c4,
						ait.d1, ait.d2, ait.d3, ait.d4);
}

AbstractTransformablePtr AssimpImporter::traverseScene(const aiNode *n)
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

void AssimpImporter::parseMesh(aiMesh *aim)
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

void AssimpImporter::parseMaterial(aiMaterial *aim)
{
	MaterialPtr mat = std::make_shared<Material>();
	float specint{0}, specrough{1};
	uint size(3);

	float diff[3];
	float spec[3];
	read<float, 3>(aim, AI_MATKEY_COLOR_DIFFUSE, diff);
	read<float, 3>(aim, AI_MATKEY_COLOR_SPECULAR, spec);
	//read<float, 1>(aim, AI_MATKEY_SHININESS, &specrough);

	glm::vec4 col(diff[0], diff[1], diff[2], 1);
	glm::vec3 sp(spec[0], spec[1], spec[2]);
	specint = (spec[0] + spec[1] + spec[2]) / 3.0f;
	mat->setProperty("diffuse_color", col);
	mat->setProperty("specular_intensity", specint);
	mat->setProperty("specular_roughness", specrough);
	materials_.push_back(mat);
}
