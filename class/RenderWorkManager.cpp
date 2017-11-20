#include "RenderWorkManager.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"

bool RenderWorkManager::Init()
{
	GetDefaultRenderPass();
	GetDefaultOffscreenRenderPass();
	return true;
}

std::shared_ptr<RenderPass> RenderWorkManager::GetDefaultRenderPass()
{
	if (m_pDefaultRenderPass)
		return m_pDefaultRenderPass;

	ASSERTION(m_pDefaultRenderPass = DefaultRenderPass());
	return m_pDefaultRenderPass;
}

std::shared_ptr<RenderPass> RenderWorkManager::GetDefaultOffscreenRenderPass()
{
	if (m_pDefaultOffScreenRenderPass)
		return m_pDefaultOffScreenRenderPass;

	ASSERTION(m_pDefaultOffScreenRenderPass = RenderPass::CreateDefaultOffscreenRenderPass());
	return m_pDefaultOffScreenRenderPass;
}