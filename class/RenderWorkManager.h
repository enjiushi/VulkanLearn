#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"

class FrameBuffer;
class Texture2D;

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

protected:
	std::shared_ptr<RenderPass>		m_pCurrentRenderPass;
	std::shared_ptr<FrameBuffer>	m_pCurrentFrameBuffer;

	FrameBuffers					m_frameBuffers[RenderMode::RenderModeCount];

	std::shared_ptr<Texture2D>		m_gbuffers[GBuffer::GBufferCount];

	RenderState						m_renderState;
	RenderMode						m_renderMode;
};