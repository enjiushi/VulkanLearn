#pragma once

#include "RenderPassBase.h"
#include "../Maths/Matrix.h"

class ForwardRenderPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<ForwardRenderPass>& pSelf, VkFormat format, VkImageLayout layout);

public:
	static std::shared_ptr<ForwardRenderPass> CreateForwardScreen();
	static std::shared_ptr<ForwardRenderPass> CreateForwardOffScreen(VkFormat format, VkImageLayout layout);

public:
	std::vector<VkClearValue> GetClearValue() override;

protected:
	Vector2ui m_size;
};