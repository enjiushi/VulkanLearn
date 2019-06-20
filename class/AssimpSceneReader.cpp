#include "AssimpSceneReader.h"
#include "postprocess.h"
#include "../common/Macros.h"
#include "Mesh.h"
#include "UniformData.h"
#include "../Base/BaseObject.h"
#include "../Maths/AssimpDataConverter.h"
#include "SkeletonAnimation.h"
#include <string>
#include <codecvt>
#include <locale>

std::vector<std::shared_ptr<Mesh>> AssimpSceneReader::Read(const std::string& path, const std::vector<uint32_t>& argumentedVAFList)
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

std::shared_ptr<Mesh> AssimpSceneReader::Read(const std::string& path, const std::vector<uint32_t>& argumentedVAFList, uint32_t meshIndex)
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

std::shared_ptr<BaseObject> AssimpSceneReader::ReadAndAssemblyScene(const std::string& path, const std::vector<uint32_t>& argumentedVAFList, std::vector<MeshLink>& outputMeshLinks)
{
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;
	pScene = imp.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	ASSERTION(pScene != nullptr);

	ExtractAnimations(pScene);

	return AssemblyNode(pScene->mRootNode, pScene, argumentedVAFList, outputMeshLinks);
}

std::shared_ptr<BaseObject> AssimpSceneReader::AssemblyNode(const aiNode* pAssimpNode, const aiScene* pScene, const std::vector<uint32_t>& argumentedVAFList, std::vector<MeshLink>& outputMeshLinks)
{
	if (pAssimpNode == nullptr)
		return nullptr;

	std::shared_ptr<BaseObject> pObject = BaseObject::Create();

	pObject->SetRotation(AssimpDataConverter::AcquireRotationMatrix(pAssimpNode->mTransformation));
	pObject->SetPos(AssimpDataConverter::AcquireTranslationVector(pAssimpNode->mTransformation));

	std::wstring wstr_name = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pAssimpNode->mName.C_Str());
	pObject->SetName(wstr_name);

	for (int32_t i = 0; i < pAssimpNode->mNumMeshes; i++)
	{
		std::shared_ptr<Mesh> pMesh = nullptr;

		// Iterate all argumented vertex format, from first to last, and see if we can get one match
		for (auto vaf : argumentedVAFList)
		{
			pMesh = Mesh::Create(pScene->mMeshes[pAssimpNode->mMeshes[i]], vaf);
			if (pMesh)
				break;
		}

		// Add mesh to result vector if available
		if (pMesh)
			outputMeshLinks.push_back({ pMesh, pObject });
	}

	for (uint32_t i = 0; i < pAssimpNode->mNumChildren; i++)
	{
		std::shared_ptr<BaseObject> pChild = AssemblyNode(pAssimpNode->mChildren[i], pScene, argumentedVAFList, outputMeshLinks);
		pObject->AddChild(pChild);
	}

	return pObject;
}

void AssimpSceneReader::ExtractAnimations(const aiScene* pScene)
{
	if (pScene->mNumAnimations == 0)
		return;

	std::shared_ptr<SkeletonAnimation> pAnimation = SkeletonAnimation::Create(pScene);

	// For temp
	uint32_t index = 0;
}

DualQuaternionf AssimpSceneReader::ExtractBoneInfo(const aiBone* pBone)
{
	return AssimpDataConverter::AcquireDualQuaternion(pBone->mOffsetMatrix);
}