#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialInstance.h"
#include <mutex>
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
#include "../vulkan/RenderWorkManager.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../class/PerObjectBuffer.h"
#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/VulkanGlobal.h"

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
	PerObjectBuffer::GetInstance()->FreePreObjectChunk(m_perObjectBufferIndex);
}

bool MeshRenderer::Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh, const std::vector<std::shared_ptr<MaterialInstance>>& materialInstances)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	m_pMesh = pMesh;
	m_materialInstances = materialInstances;

	m_perObjectBufferIndex = PerObjectBuffer::GetInstance()->AllocatePerObjectChunk();
	return true;
}

void MeshRenderer::Update()
{
}

void MeshRenderer::LateUpdate()
{

}

void MeshRenderer::Draw(const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{
	std::unique_lock<std::mutex> lock(m_updateMutex);

	for (uint32_t i = 0; i < m_materialInstances.size(); i++)
	{
		if (((1 << GetGlobalVulkanStates()->GetRenderState()) & m_materialInstances[i]->GetRenderMask()) == 0)
			continue;

		std::shared_ptr<CommandBuffer> pDrawCmdBuffer = pPerFrameRes->AllocateSecondaryCommandBuffer();

		std::vector<VkClearValue> clearValues =
		{
			{ 0.2f, 0.2f, 0.2f, 0.2f },
			{ 1.0f, 0 }
		};

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = RenderWorkMgr()->GetCurrentRenderPass()->GetDeviceHandle();
		inheritanceInfo.subpass = RenderWorkMgr()->GetCurrentRenderPass()->GetCurrentSubpass();
		inheritanceInfo.framebuffer = RenderWorkMgr()->GetCurrentFrameBuffer()->GetDeviceHandle();
		pDrawCmdBuffer->StartSecondaryRecording(inheritanceInfo);

		VkViewport viewport =
		{
			0, 0,
			RenderWorkMgr()->GetCurrentFrameBuffer()->GetFramebufferInfo().width, RenderWorkMgr()->GetCurrentFrameBuffer()->GetFramebufferInfo().height,
			0, 1
		};

		VkRect2D scissorRect =
		{
			0, 0,
			RenderWorkMgr()->GetCurrentFrameBuffer()->GetFramebufferInfo().width, RenderWorkMgr()->GetCurrentFrameBuffer()->GetFramebufferInfo().height,
		};

		pDrawCmdBuffer->SetViewports({ GetGlobalVulkanStates()->GetViewport() });
		pDrawCmdBuffer->SetScissors({ GetGlobalVulkanStates()->GetScissorRect() });

		m_materialInstances[i]->PrepareMaterial(pDrawCmdBuffer);
		m_pMesh->PrepareMeshData(pDrawCmdBuffer);

		pDrawCmdBuffer->DrawIndexed(m_pMesh->GetIndexBuffer());

		pDrawCmdBuffer->EndSecondaryRecording();

		RenderWorkMgr()->GetCurrentRenderPass()->CacheSecondaryCommandBuffer(pDrawCmdBuffer);
	}
}