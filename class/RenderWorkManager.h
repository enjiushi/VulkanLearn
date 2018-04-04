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
		RenderStateCount
	};

public:
	bool Init();

public:
	void SetRenderState(RenderState renderState) { m_renderState = renderState; }
	RenderState GetRenderState() const { return m_renderState; }

protected:
	RenderState						m_renderState;
};