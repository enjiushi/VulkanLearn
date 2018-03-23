#include "RenderPassDiction.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "ForwardRenderPass.h"
#include "DeferredRenderPass.h"

bool RenderPassDiction::Init()
{
	if (!Singleton<RenderPassDiction>::Init())
		return false;

	m_pForwardRenderPass = ForwardRenderPass::Create(GetDevice()->GetPhysicalDevice()->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	m_pForwardRenderPassOffScreen = ForwardRenderPass::Create(RenderPassBase::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	m_pDeferredRenderPass = DeferredRenderPass::Create(GetDevice()->GetPhysicalDevice()->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	return true;
}