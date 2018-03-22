#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include <map>

class RenderPassDiction : public Singleton<RenderPassDiction>
{
public:
	static const VkFormat OFFSCREEN_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormat OFFSCREEN_HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat OFFSCREEN_DEPTH_STENCIL_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;
	static const VkFormat GBUFFER0_COLOR_FORMAT = VK_FORMAT_A2R10G10B10_UNORM_PACK32;

public:
	typedef std::pair<uint32_t, uint32_t> RenderPassDependency;

public:
	bool Init() override;

public:
	std::shared_ptr<RenderPass> GetDefaultRenderPass();
	std::shared_ptr<RenderPass> GetDefaultOffScreenRenderPass();
	std::shared_ptr<RenderPass> GetRGBA8x4_D24S8DeferredRenderPass();	// 0: screen frame buffer

protected:
	static std::shared_ptr<RenderPass> CreateDefaultRenderPass();
	static std::shared_ptr<RenderPass> CreateDefaultOffScreenRenderPass();
	static std::shared_ptr<RenderPass> CreateRGBA8x3_D24S8DeferredRenderPass();

protected:
	std::shared_ptr<RenderPass>			m_pDefaultRenderPass;
	std::shared_ptr<RenderPass>			m_pDefaultOffScreenRenderPass;
	std::shared_ptr<RenderPass>			m_pRGBA8x3_D24S8RenderPass;
};