#pragma once
#include "../Base/BaseComponent.h"

class Mesh;
class Material;
class MaterialInstance;
class DescriptorSet;
class DescriptorPool;

class MeshRenderer : public BaseComponent
{
public:
	static std::shared_ptr<MeshRenderer> Create(const std::shared_ptr<Mesh> pMesh, const std::shared_ptr<MaterialInstance>& pMaterialInstance);
	static std::shared_ptr<MeshRenderer> Create();

public:
	void Update(const std::shared_ptr<PerFrameResource>& pPerFrameRes) override;
	void LateUpdate(const std::shared_ptr<PerFrameResource>& pPerFrameRes) override;

protected:
	bool Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh, const std::shared_ptr<MaterialInstance>& pMaterialInstance);

protected:
	std::shared_ptr<Mesh>				m_pMesh;
	std::shared_ptr<MaterialInstance>	m_pMaterialInstance;
};