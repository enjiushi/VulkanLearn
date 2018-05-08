#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include "RenderPassDiction.h"

class FrameBuffer;
class Texture2D;
class DepthStencilBuffer;
class GBufferMaterial;
class ShadowMapMaterial;
class SSAOMaterial;
class GaussianBlurMaterial;
class DeferredShadingMaterial;
class ForwardMaterial;
class BloomMaterial;
class PostProcessingMaterial;

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

	void Draw();

protected:
	uint32_t m_renderStateMask;
	
	std::shared_ptr<GBufferMaterial>			m_PBRGbufferMaterial;
	std::shared_ptr<ShadowMapMaterial>			m_pShadowMapMaterial;
	std::shared_ptr<SSAOMaterial>				m_pSSAOMaterial;
	std::shared_ptr<GaussianBlurMaterial>		m_pSSAOBlurVMaterial;
	std::shared_ptr<GaussianBlurMaterial>		m_pSSAOBlurHMaterial;
	std::shared_ptr<DeferredShadingMaterial>	m_pShadingMaterial;
	std::shared_ptr<ForwardMaterial>			m_pSkyBoxMaterial;
	std::shared_ptr<BloomMaterial>				m_pBloomMaterial;
	std::shared_ptr<GaussianBlurMaterial>		m_pBloomBlurVMaterial;
	std::shared_ptr<GaussianBlurMaterial>		m_pBloomBlurHMaterial;
	std::shared_ptr<PostProcessingMaterial>		m_pPostProcessMaterial;
};