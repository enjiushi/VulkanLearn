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
#include "../vulkan/VulkanGlobal.h"
#include "../vulkan/StagingBufferManager.h"

std::shared_ptr<MeshRenderer> MeshRenderer::Create(const std::shared_ptr<Mesh> pMesh, const std::shared_ptr<MaterialInstance>& pMaterialInstance)
{
	std::shared_ptr<MeshRenderer> pMeshRenderer = std::make_shared<MeshRenderer>();
	if (pMeshRenderer.get() && pMeshRenderer->Init(pMeshRenderer, pMesh, pMaterialInstance))
		return pMeshRenderer;
	return nullptr;
}

std::shared_ptr<MeshRenderer> MeshRenderer::Create()
{
	std::shared_ptr<MeshRenderer> pMeshRenderer = std::make_shared<MeshRenderer>();
	if (pMeshRenderer.get() && pMeshRenderer->Init(pMeshRenderer, nullptr, nullptr))
		return pMeshRenderer;
	return nullptr;
}

bool MeshRenderer::Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh, const std::shared_ptr<MaterialInstance>& pMaterialInstance)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	m_pMesh = pMesh;
	m_pMaterialInstance = pMaterialInstance;
	return true;
}

void MeshRenderer::Update(const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{
	std::unique_lock<std::mutex> lock(m_updateMutex);

	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = pPerFrameRes->AllocateCommandBuffer();

	std::vector<VkClearValue> clearValues =
	{
		{ 0.2f, 0.2f, 0.2f, 0.2f },
		{ 1.0f, 0 }
	};

	pDrawCmdBuffer->StartRecording();

	VulkanGlobal::GetInstance()->UpdateUniforms(pPerFrameRes->GetFrameIndex(), VulkanGlobal::GetInstance()->m_pCameraComp);
	StagingBufferMgr()->RecordDataFlush(pDrawCmdBuffer);

	uint32_t totalUniformBytes = VulkanGlobal::GetInstance()->m_pUniformBuffer->GetDescBufferInfo().range / GetSwapChain()->GetSwapChainImageCount();
	VulkanGlobal::GetInstance()->m_pUniformBuffer->UpdateByteStream(&VulkanGlobal::GetInstance()->m_globalUniforms, totalUniformBytes * pPerFrameRes->GetFrameIndex(), sizeof(VulkanGlobal::m_globalUniforms));
	StagingBufferMgr()->RecordDataFlush(pDrawCmdBuffer);

	VkViewport viewport =
	{
		0, 0,
		GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height
	};

	pDrawCmdBuffer->SetViewports({ viewport });
	pDrawCmdBuffer->SetScissors({ scissorRect });

	uint32_t offset = FrameMgr()->FrameIndex() * VulkanGlobal::GetInstance()->m_pUniformBuffer->GetDescBufferInfo().range / GetSwapChain()->GetSwapChainImageCount();

	pDrawCmdBuffer->BeginRenderPass(GlobalObjects()->GetCurrentFrameBuffer(), clearValues);

	// Draw gun
	pDrawCmdBuffer->BindDescriptorSets(m_pMaterialInstance->GetMaterial()->GetPipelineLayout(), m_pMaterialInstance->GetDescriptorSets(), { offset });
	pDrawCmdBuffer->BindPipeline(m_pMaterialInstance->GetMaterial()->GetGraphicPipeline());
	pDrawCmdBuffer->BindVertexBuffers({ m_pMesh->GetVertexBuffer() });
	pDrawCmdBuffer->BindIndexBuffer(m_pMesh->GetIndexBuffer());
	pDrawCmdBuffer->DrawIndexed(m_pMesh->GetIndexBuffer());

	// Draw skybox
	pDrawCmdBuffer->BindDescriptorSets(VulkanGlobal::GetInstance()->m_pSkyBoxPLayout, { VulkanGlobal::GetInstance()->m_pSkyBoxDS }, { offset });
	pDrawCmdBuffer->BindPipeline(VulkanGlobal::GetInstance()->m_pSkyBoxPipeline);
	pDrawCmdBuffer->BindVertexBuffers({ VulkanGlobal::GetInstance()->m_pCubeMesh->GetVertexBuffer() });
	pDrawCmdBuffer->BindIndexBuffer(VulkanGlobal::GetInstance()->m_pCubeMesh->GetIndexBuffer());

	pDrawCmdBuffer->DrawIndexed(VulkanGlobal::GetInstance()->m_pCubeMesh->GetIndexBuffer());

	pDrawCmdBuffer->EndRenderPass();

	pDrawCmdBuffer->EndRecording();

	std::vector<VkPipelineStageFlags> waitFlags = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	FrameMgr()->CacheSubmissioninfo(GlobalGraphicQueue(), { pDrawCmdBuffer }, waitFlags, false);
}

void MeshRenderer::LateUpdate(const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{

}