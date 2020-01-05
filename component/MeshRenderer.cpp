#include "MeshRenderer.h"
#include "../class/Mesh.h"
#include "Material.h"
#include "../class/MaterialInstance.h"
#include <mutex>
#include "../Base/BaseObject.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/PhysicalDevice.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/SharedVertexBuffer.h"
#include "../vulkan/SharedIndexBuffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/GraphicPipeline.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/StagingBufferManager.h"
#include "../class/RenderWorkManager.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/Framebuffer.h"
#include "../class/UniformData.h"
#include "../class/Material.h"

DEFINITE_CLASS_RTTI(MeshRenderer, BaseComponent);

std::shared_ptr<MeshRenderer> MeshRenderer::Create(const std::shared_ptr<Mesh> pMesh, const std::shared_ptr<MaterialInstance>& pMaterialInstance)
{
	std::shared_ptr<MeshRenderer> pMeshRenderer = std::make_shared<MeshRenderer>();
	if (pMeshRenderer.get() && pMeshRenderer->Init(pMeshRenderer, pMesh, { pMaterialInstance }))
		return pMeshRenderer;
	return nullptr;
}

std::shared_ptr<MeshRenderer> MeshRenderer::Create(const std::shared_ptr<Mesh> pMesh, const std::vector<std::shared_ptr<MaterialInstance>>& materialInstances)
{
	std::shared_ptr<MeshRenderer> pMeshRenderer = std::make_shared<MeshRenderer>();
	if (pMeshRenderer.get() && pMeshRenderer->Init(pMeshRenderer, pMesh, materialInstances))
		return pMeshRenderer;
	return nullptr;
}

std::shared_ptr<MeshRenderer> MeshRenderer::Create()
{
	std::shared_ptr<MeshRenderer> pMeshRenderer = std::make_shared<MeshRenderer>();
	if (pMeshRenderer.get() && pMeshRenderer->Init(pMeshRenderer, nullptr, {}))
		return pMeshRenderer;
	return nullptr;
}

MeshRenderer::~MeshRenderer()
{
	UniformData::GetInstance()->GetPerObjectUniforms()->FreePreObjectChunk(m_perObjectBufferIndex);
}

bool MeshRenderer::Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh, const std::vector<std::shared_ptr<MaterialInstance>>& materialInstances)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	m_pMesh = pMesh;

	for (auto & val : materialInstances)
	{
		m_materialInstances.push_back(val);

#if defined(_DEBUG)
		if (m_pMesh != nullptr)
			ASSERTION(m_pMesh->GetVertexFormat() == val->GetMaterial()->GetVertexFormatInMem());
#endif
	}

	m_perObjectBufferIndex = UniformData::GetInstance()->GetPerObjectUniforms()->AllocatePerObjectChunk();

	return true;
}

void MeshRenderer::OnRenderObject()
{
	if (m_pMesh == nullptr)
		return;

	// Don't render anything
	if (m_instanceCount == 0)
		return;

	if (m_modelMatrixOverride)
	{
		UniformData::GetInstance()->GetPerObjectUniforms()->SetModelMatrix(m_perObjectBufferIndex, m_overrideModelMatrix);
		m_modelMatrixOverride = false;
	}
	else
		UniformData::GetInstance()->GetPerObjectUniforms()->SetModelMatrix(m_perObjectBufferIndex, GetBaseObject()->GetCachedWorldTransform());

	for (uint32_t i = 0; i < m_materialInstances.size(); i++)
	{
		if ((RenderWorkManager::GetInstance()->GetRenderStateMask() & m_materialInstances[i]->GetRenderMask()) == 0)
			continue;

		m_materialInstances[i]->InsertIntoRenderQueue(m_pMesh, m_perObjectBufferIndex, m_pMesh->GetMeshChunkIndex(), m_utilityIndex, m_instanceCount, m_startInstance);
	}
}