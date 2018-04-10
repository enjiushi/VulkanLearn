#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include "RenderPassDiction.h"

class FrameBuffer;
class Texture2D;
class DepthStencilBuffer;

class RenderWorkManager : public Singleton<RenderWorkManager>
{
public:
	enum RenderState
	{
		None,
		IrradianceGen,
		ReflectionGen,
		BrdfLutGen,
		Scene,
		ShadowMapGen,
		RenderStateCount
	};

public:
	bool Init();

public:
	void SetRenderStateMask(RenderState renderState) { m_renderStateMask = (1 << renderState); }
	void SetRenderStateMask(uint32_t mask) { m_renderStateMask = mask; }
	void AddRenderStateMask(RenderState renderState) { m_renderStateMask |= (1 << renderState); }
	uint32_t GetRenderStateMask() const { return m_renderStateMask; }

protected:
	uint32_t	m_renderStateMask;
};