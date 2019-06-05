#include "AssimpSceneReader.h"
#include "postprocess.h"
#include "../common/Macros.h"
#include "Mesh.h"

std::vector<std::shared_ptr<Mesh>> AssimpSceneReader::Read(const std::string& path, std::vector<uint32_t> argumentedVAFList)
{
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;
	pScene = imp.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	ASSERTION(pScene != nullptr);

	std::vector<std::shared_ptr<Mesh>> meshes;
	for (int32_t i = 0; i < pScene->mNumMeshes; i++)
	{
		std::shared_ptr<Mesh> pMesh = nullptr;

		// Iterate all argumented vertex format, from first to last, and see if we can get one match
		for (auto vaf : argumentedVAFList)
		{
			pMesh = Mesh::Create(pScene->mMeshes[i], vaf);
			if (pMesh)
				break;
		}

		// Add mesh to result vector if available
		if (pMesh)
			meshes.push_back(pMesh);
	}
	return meshes;
}

std::shared_ptr<Mesh> AssimpSceneReader::Read(const std::string& path, std::vector<uint32_t> argumentedVAFList, uint32_t meshIndex)
{
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;
	pScene = imp.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	ASSERTION(pScene != nullptr && meshIndex < pScene->mNumMeshes);

	std::shared_ptr<Mesh> pMesh = nullptr;

	// Iterate all argumented vertex format, from first to last, and see if we can get one match
	for (auto vaf : argumentedVAFList)
	{
		pMesh = Mesh::Create(pScene->mMeshes[meshIndex], vaf);
		if (pMesh)
			break;
	}

	return pMesh;
}