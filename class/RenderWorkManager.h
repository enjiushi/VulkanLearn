#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"

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

	enum RenderMode
	{
		Forward,
		Deferred,
		RenderModeCount
	};

	enum GBuffer
	{
		GBuffer0,
		GBuffer1,
		GBuffer2,
		GBufferCount
	};

	typedef std::vector<std::shared_ptr<FrameBuffer>> FrameBuffers;

public:
	bool Init();

public:
	std::shared_ptr<RenderPass> GetCurrentRenderPass() const { return m_pCurrentRenderPass; }
	std::shared_ptr<FrameBuffer> GetCurrentFrameBuffer() const { return m_pCurrentFrameBuffer; }

	void SetCurrentFrameBuffer(const std::shared_ptr<FrameBuffer>& pFrameBuffer);
	void SetCurrentFrameBuffer(RenderMode mode);
	void SetRenderState(RenderState renderState) { m_renderState = renderState; }
	RenderState GetRenderState() const { return m_renderState; }

	std::vector<std::shared_ptr<Texture2D>> GetGBuffers() const { std::vector<std::shared_ptr<Texture2D>> vec(m_gbuffers, m_gbuffers + GBuffer::GBufferCount); return vec; }
	std::shared_ptr<Texture2D> GetGBuffer(uint32_t index) const { return m_gbuffers[index]; }

	std::shared_ptr<DepthStencilBuffer> GetDeferredDepthStencilBuffer() const { return m_pDepthStencilBuffer; }

protected:
	std::shared_ptr<RenderPass>		m_pCurrentRenderPass;
	std::shared_ptr<FrameBuffer>	m_pCurrentFrameBuffer;

	FrameBuffers					m_frameBuffers[RenderMode::RenderModeCount];

	std::shared_ptr<Texture2D>			m_gbuffers[GBuffer::GBufferCount];
	std::shared_ptr<DepthStencilBuffer>	m_pDepthStencilBuffer;

	RenderState						m_renderState;
	RenderMode						m_renderMode;
};