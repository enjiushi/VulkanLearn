#pragma once

#include "RenderPassBase.h"

class DeferredRenderPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<DeferredRenderPass>& pSelf, VkFormat format, VkImageLayout layout);

public:
	static std::shared_ptr<DeferredRenderPass> Create(VkFormat format, VkImageLayout layout);

public:
	std::vector<VkClearValue> GetClearValue() override;

	void BeginGeometryPass();
	void EndGeometryPass();

	void BeginShadingPass();
	void EndShadingPass();

	void BeginTransparentPass();
	void EndTransparentPass();
};