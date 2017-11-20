#include "DeviceObjectBase.h"

class RenderPass;
class FrameBuffer;

class RenderWorkManager : DeviceObjectBase<RenderWorkManager>
{
public:
	static std::shared_ptr<RenderWorkManager> Create(const std::shared_ptr<Device>& pDevice);

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<RenderWorkManager>& pSelf);

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

protected:
	std::shared_ptr<RenderPass>		m_pCurrentRenderPass;
	std::shared_ptr<FrameBuffer>	m_pCurrentFrameBuffer;

public:
	static std::shared_ptr<RenderPass> GetDefaultRenderPass();
	static bool CreateDefaultRenderPass();

	static std::shared_ptr<RenderPass> GetDefaultOffscreenRenderPass();
	static bool CreateDefaultOffscreenRenderPass();


	static std::shared_ptr<RenderPass>	m_pDefaultRenderPass;
	static std::shared_ptr<RenderPass>	m_pDefaultOffScreenRenderPass;
};