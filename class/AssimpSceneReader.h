#pragma once
#include "../common/Singleton.h"
#include "Importer.hpp"
#include "scene.h"
#include <vector>
#include <memory>
#include "../Maths/DualQuaternion.h"

class Mesh;
class BaseObject;
class SkeletonAnimation;

class AssimpSceneReader : public Singleton<AssimpSceneReader>
{
public:
	typedef std::pair<std::shared_ptr<Mesh>, std::shared_ptr<BaseObject>> MeshLink;
	typedef struct _SceneInfo
	{
		std::vector<MeshLink>				meshLinks;
		std::shared_ptr<SkeletonAnimation>	pAnimation;
	}SceneInfo;

public:
	static std::vector<std::shared_ptr<Mesh>> Read(const std::string& path, const std::vector<uint32_t>& argumentedVAFList);
	static std::shared_ptr<Mesh> Read(const std::string& path, const std::vector<uint32_t>& argumentedVAFList, uint32_t meshIndex);
	static std::shared_ptr<BaseObject> ReadAndAssemblyScene(const std::string& path, const std::vector<uint32_t>& argumentedVAFList, SceneInfo& sceneInfo);

protected:
	static void ExtractAnimations(const aiScene* pScene);
	static DualQuaternionf ExtractBoneInfo(const aiBone* pBone);
	static std::shared_ptr<BaseObject> AssemblyNode(const aiNode* pAssimpNode, const aiScene* pScene, const std::vector<uint32_t>& argumentedVAFList, SceneInfo& sceneInfo);
};