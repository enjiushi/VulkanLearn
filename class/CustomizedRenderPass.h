#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class CustomizedRenderPass : public RenderPassBase
{
public:
	typedef struct _RenderPassAttachDesc
	{
		VkFormat		format;
		VkImageLayout	initialLayout;
		VkImageLayout	finalLayout;
		VkClearValue	clearValue;
	}RenderPassAttachDesc;

protected:
	bool Init(const std::shared_ptr<CustomizedRenderPass>& pSelf, const std::vector<RenderPassAttachDesc>& attachList);

public:
	static std::shared_ptr<CustomizedRenderPass> Create(const std::vector<RenderPassAttachDesc>& attachList);

public:
	std::vector<VkClearValue> GetClearValue() override;

protected:
	std::vector<VkClearValue>	m_clearColors;
};