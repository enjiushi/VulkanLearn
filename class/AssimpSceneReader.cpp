#include "AssimpSceneReader.h"
#include "postprocess.h"
#include "../common/Macros.h"
#include "Mesh.h"
#include "UniformData.h"

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

	ExtractAnimations(path);

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

	ExtractAnimations(path);

	return pMesh;
}

void AssimpSceneReader::ExtractAnimations(const std::string& path)
{
	Assimp::Importer imp;
	const aiScene* pScene = imp.ReadFile(path.c_str(), 0);

	if (pScene->mNumAnimations == 0)
		return;

	// For temp
	uint32_t index = 0;

	for (uint32_t meshIndex = 0; meshIndex < pScene->mNumMeshes; meshIndex++)
	{
		for (uint32_t boneIndex = 0; boneIndex < pScene->mMeshes[meshIndex]->mNumBones; boneIndex++)
		{
			DualQuaternionf dq = ExtractBoneInfo(pScene->mMeshes[meshIndex]->mBones[boneIndex]);
			UniformData::GetInstance()->GetPerFrameBoneUniforms()->SetAnimationTransform(index, dq);
			UniformData::GetInstance()->GetPerFrameBoneUniforms()->SetReferenceTransform(index++, dq);
		}
	}
}

DualQuaternionf AssimpSceneReader::ExtractBoneInfo(const aiBone* pBone)
{
	const aiMatrix4x4 pReferenceMatrix = pBone->mOffsetMatrix;

	Matrix3f rotation(
		pReferenceMatrix.a1, pReferenceMatrix.b1, pReferenceMatrix.c1,
		pReferenceMatrix.a2, pReferenceMatrix.b2, pReferenceMatrix.c2,
		pReferenceMatrix.a3, pReferenceMatrix.b3, pReferenceMatrix.c3);

	Vector3f translate(pReferenceMatrix.a4, pReferenceMatrix.b4, pReferenceMatrix.c4);

	return DualQuaternionf(rotation.AcquireQuaternion(), translate);
}