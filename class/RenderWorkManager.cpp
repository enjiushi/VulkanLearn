#include "RenderWorkManager.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "../vulkan/Texture2D.h"
#include "RenderPassDiction.h"

bool RenderWorkManager::Init()
{
	if (!Singleton<RenderWorkManager>::Init())
		return false;

	m_renderMode = Forward;	// FIXME: should put this into some configuration class

	for (auto& fbs : m_frameBuffers)
	{
		fbs.resize(GetSwapChain()->GetSwapChainImageCount());
	}

	for (uint32_t i = 0; i < GBuffer::GBufferCount; i++)
	{
		m_gbuffers[i] = Texture2D::CreateOffscreenTexture(GetDevice(), GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.width, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.height, VK_FORMAT_R8G8B8A8_UNORM);
	}

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		m_frameBuffers[RenderMode::Forward][i] = FrameBuffer::Create(GetDevice(), GetSwapChain()->GetSwapChainImage(i), DepthStencilBuffer::Create(GetDevice()), RenderPassDiction::GetInstance()->GetDefaultRenderPass());
		
		std::vector<std::shared_ptr<Image>> temp_vec;
		temp_vec.insert(temp_vec.end(), GetSwapChain()->GetSwapChainImage(i));
		temp_vec.insert(temp_vec.end(), m_gbuffers, m_gbuffers + GBuffer::GBufferCount);
		m_frameBuffers[RenderMode::Deferred][i] = FrameBuffer::Create(GetDevice(), temp_vec, DepthStencilBuffer::Create(GetDevice()), RenderPassDiction::GetInstance()->GetRGBA8x4_D24S8DeferredRenderPass());
	}

	return true;
}

void RenderWorkManager::SetCurrentFrameBuffer(const std::shared_ptr<FrameBuffer>& pFrameBuffer)
{
	m_pCurrentFrameBuffer = pFrameBuffer;
	m_pCurrentRenderPass = m_pCurrentFrameBuffer->GetRenderPass();
}

void RenderWorkManager::SetCurrentFrameBuffer(RenderMode mode)
{
	m_pCurrentFrameBuffer = m_frameBuffers[mode][FrameMgr()->FrameIndex()];
	m_pCurrentRenderPass = m_pCurrentFrameBuffer->GetRenderPass();
}