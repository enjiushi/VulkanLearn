#include "GlobalVulkanStates.h"

std::shared_ptr<GlobalVulkanStates> GlobalVulkanStates::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<GlobalVulkanStates> pStates = std::make_shared<GlobalVulkanStates>();
	if (pStates.get() && pStates->Init(pDevice, pStates))
		return pStates;
	return nullptr;
}

bool GlobalVulkanStates::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<GlobalVulkanStates>& pStates)
{
	if (!DeviceObjectBase<GlobalVulkanStates>::Init(pDevice, pStates))
		return false;

	RestoreViewport();
	RestoreScissor();

	return true;
}

void GlobalVulkanStates::RestoreViewport()
{
	VkViewport viewport =
	{
		0, 0,
		(float)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, (float)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height,
		0, 1
	};

	SetViewport(viewport);
}

void GlobalVulkanStates::RestoreScissor()
{
	VkRect2D scissorRect =
	{
		0, 0,
		GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height,
	};

	SetScissorRect(scissorRect);
}