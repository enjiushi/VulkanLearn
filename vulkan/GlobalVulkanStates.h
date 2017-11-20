#include "DeviceObjectBase.h"

class GlobalVulkanStates : public DeviceObjectBase<GlobalVulkanStates>
{
public:
	void SetViewport(const VkViewport& viewport) { m_viewport = viewport; }
	VkViewport GetViewport() const { return m_viewport; }

	void SetScissorRect(const VkRect2D& rect) { m_scissorRect = rect; }
	VkRect2D GetScissorRect() const { return m_scissorRect; }

	void RestoreViewport();
	void RestoreScissor();

public:
	static std::shared_ptr<GlobalVulkanStates> Create(const std::shared_ptr<Device>& pDevice);

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<GlobalVulkanStates>& pStates);

protected:
	VkViewport	m_viewport;
	VkRect2D	m_scissorRect;
};