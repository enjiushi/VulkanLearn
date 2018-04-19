#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"

class GaussianBlurMaterial : public Material
{
public:
	static std::shared_ptr<GaussianBlurMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo, 
		FrameBufferDiction::FrameBufferType vertical, 
		FrameBufferDiction::FrameBufferType horizontal);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;

protected:
	FrameBufferDiction::FrameBufferType m_currentFB;
	FrameBufferDiction::FrameBufferType	m_verticalBlurFB;
	FrameBufferDiction::FrameBufferType	m_horizontalBlurFB;
};