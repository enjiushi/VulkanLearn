#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

class Mesh;
class Material;
class MaterialInstance;
class DescriptorSet;
class DescriptorPool;

class MeshRenderer : public BaseComponent
{
	DECLARE_CLASS_RTTI(MeshRenderer);

public:
	static std::shared_ptr<MeshRenderer> Create(const std::shared_ptr<Mesh> pMesh, const std::shared_ptr<MaterialInstance>& pMaterialInstance);
	static std::shared_ptr<MeshRenderer> Create(const std::shared_ptr<Mesh> pMesh, const std::vector<std::shared_ptr<MaterialInstance>>& materialInstances);
	static std::shared_ptr<MeshRenderer> Create();

	~MeshRenderer();

public:
	void OnRenderObject() override;

	std::shared_ptr<Mesh> GetMesh() const { return m_pMesh; }

	uint32_t GetInstanceCount() const { return m_instanceCount; }
	void SetInstanceCount(uint32_t instanceCount) { m_instanceCount = instanceCount; }
	uint32_t GetStartInstance() const { return m_startInstance; }
	void SetStartInstance(uint32_t startInstance) { m_startInstance = startInstance; }
	uint32_t GetUtilityIndex() const { return m_utilityIndex; }
	void SetUtilityIndex(uint32_t index) { m_utilityIndex = index; }
	void OverrideModelMatrix(const Matrix4d& matrix) { m_overrideModelMatrix = matrix; m_modelMatrixOverride = true; }

protected:
	bool Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh, const std::vector<std::shared_ptr<MaterialInstance>>& materialInstances);

protected:
	std::shared_ptr<Mesh>	m_pMesh;
	uint32_t				m_perObjectBufferIndex;

	std::vector<std::shared_ptr<MaterialInstance>> m_materialInstances;

	// For the same mesh with same material, there's a mechanism to get them rendered with instancing rather than multi indirect command
	// And a special procedure is invented to use both "DrawID" and "InstanceID" to redirect to the right per-object data chunk
	// However, when it comes to the need of rendering something with a customized amount of instances, the whole mechanism goes south
	//
	// Good thing is, customized instance rendering most likely do jobs as drawing many things with same geometry, and each one of them is 
	// distinguished by per-instanced vertex attribute or uniform buffer, indexed by instance id.
	// Since this works completely differently, we can have these 2 variables to let it decide to join either auto
	// instancing, or generate its own indirect command and assign instance count manually ( With 0 or 1 means auto instancing)
	//
	// Everything is default to be auto instanced;
	uint32_t				m_instanceCount = 1;
	uint32_t				m_startInstance = 0;

	uint32_t				m_utilityIndex = 0;

	bool					m_modelMatrixOverride = false;
	Matrix4d				m_overrideModelMatrix;
};