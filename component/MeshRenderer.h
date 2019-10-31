#pragma once
#include "../Base/BaseComponent.h"

class Mesh;
class Material;
class MaterialInstance;
class DescriptorSet;
class DescriptorPool;
class AnimationController;

class MeshRenderer : public BaseComponent
{
	DECLARE_CLASS_RTTI(MeshRenderer);

public:
	static std::shared_ptr<MeshRenderer> Create(const std::shared_ptr<Mesh> pMesh, const std::shared_ptr<MaterialInstance>& pMaterialInstance, const std::shared_ptr<AnimationController>& pAnimationController = nullptr);
	static std::shared_ptr<MeshRenderer> Create(const std::shared_ptr<Mesh> pMesh, const std::vector<std::shared_ptr<MaterialInstance>>& materialInstances, const std::shared_ptr<AnimationController>& pAnimationController = nullptr);
	static std::shared_ptr<MeshRenderer> Create();

	~MeshRenderer();

public:
	void Update() override;
	void LateUpdate() override;
	void Draw(const std::shared_ptr<PerFrameResource>& pPerFrameRes) override;

	std::shared_ptr<Mesh> GetMesh() const { return m_pMesh; }

	bool GetAllowAutoInstancedRendering() const { return m_allowAutoInstancedRendering; }
	void SetAllowAutoInstancedRendering(bool val) { m_allowAutoInstancedRendering = val; }

protected:
	bool Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh, const std::vector<std::shared_ptr<MaterialInstance>>& materialInstances, const std::shared_ptr<AnimationController>& pAnimationController);

protected:
	std::shared_ptr<Mesh>	m_pMesh;
	uint32_t				m_perObjectBufferIndex;

	// First: material instance
	// Second: Key reference of material instance to this mesh renderer, used in destructor to remove weak reference in material instance
	std::vector<std::pair<std::shared_ptr<MaterialInstance>, uint32_t>> m_materialInstances;

	std::shared_ptr<AnimationController>	m_pAnimationController;

	// For the same mesh with same material, there's a mechanism to get them rendered with instancing rather than multi indirect command
	// And a special procedure is invented to use both "DrawID" and "InstanceID" to redirect to the right per-object data chunk
	// However, when it comes to the need of rendering something with a customized amount of instances, the whole mechanism goes south
	//
	// Good thing is, customized instance rendering mostly likely do jobs as drawing many things with same geometry, and each one of them is 
	// distinguished by per-instanced vertex attribute or uniform buffer, indexed by instance id.
	// Since this works completely differently, we can have this variable to mark current mesh renderer and let it decide to join either auto
	// instancing, or generate its own indirect command and assign instance count manually
	//
	// Everything is default to be auto instanced
	bool					m_allowAutoInstancedRendering = true;
};