#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"

class FrameBuffer;

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
		Count
	};

public:
	bool Init();

public:
	std::shared_ptr<RenderPass> GetCurrentRenderPass() const { return m_pCurrentRenderPass; }
	std::shared_ptr<FrameBuffer> GetCurrentFrameBuffer() const { return m_pCurrentFrameBuffer; }

	void SetDefaultRenderPass(const std::shared_ptr<FrameBuffer>& pFrameBuffer) 
	{ 
		m_pCurrentRenderPass = GetDefaultRenderPass(); 
		m_pCurrentFrameBuffer = pFrameBuffer; 
	}

	void SetDefaultOffscreenRenderPass(const std::shared_ptr<FrameBuffer>& pFrameBuffer) 
	{ 
		m_pCurrentRenderPass = GetDefaultOffscreenRenderPass(); 
		m_pCurrentFrameBuffer = pFrameBuffer; 
	}

	void SetDeferredRenderPass(const std::shared_ptr<FrameBuffer>& pFrameBuffer);

	void SetRenderState(RenderState renderState) { m_renderState = renderState; }
	RenderState GetRenderState() const { return m_renderState; }

public:
	std::shared_ptr<RenderPass> GetDefaultRenderPass();
	std::shared_ptr<RenderPass> GetDefaultOffscreenRenderPass();

protected:
	std::shared_ptr<RenderPass>			m_pDefaultRenderPass;
	std::shared_ptr<RenderPass>			m_pDefaultOffScreenRenderPass;

	std::shared_ptr<RenderPass>			m_pCurrentRenderPass;
	std::shared_ptr<FrameBuffer>		m_pCurrentFrameBuffer;

	RenderState							m_renderState;
};