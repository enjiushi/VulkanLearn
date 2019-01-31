#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include "RenderPassDiction.h"

class FrameBuffer;
class Texture2D;
class DepthStencilBuffer;
class GBufferMaterial;
class MotionTileMaxMaterial;
class MotionNeighborMaxMaterial;
class ShadowMapMaterial;
class SSAOMaterial;
class GaussianBlurMaterial;
class DeferredShadingMaterial;
class ForwardMaterial;
class TemporalResolveMaterial;
class BloomMaterial;
class CombineMaterial;
class PostProcessingMaterial;
class MaterialInstance;
class CommandBuffer;
class DOFMaterial;

class RenderWorkManager : public Singleton<RenderWorkManager>
{
	// FIXME: Temp
	static const uint32_t BLOOM_ITER_COUNT = 5;

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

	std::shared_ptr<MaterialInstance> AcquirePBRMaterialInstance() const;
	std::shared_ptr<MaterialInstance> AcquireShadowMaterialInstance() const;
	std::shared_ptr<MaterialInstance> AcquireSkyBoxMaterialInstance() const;

	void SyncMaterialData();
	void Draw(const std::shared_ptr<CommandBuffer>& pDrawCmdBuffer, uint32_t pingpong);

	void OnFrameBegin();
	void OnFrameEnd();

protected:
	uint32_t m_renderStateMask;
	
	std::shared_ptr<GBufferMaterial>			m_PBRGbufferMaterial;
	std::shared_ptr<ForwardMaterial>			m_pBackgroundMotionMaterial;
	std::shared_ptr<MotionTileMaxMaterial>		m_pMotionTileMaxMaterial;
	std::shared_ptr<MotionNeighborMaxMaterial>	m_pMotionNeighborMaxMaterial;
	std::shared_ptr<ShadowMapMaterial>			m_pShadowMapMaterial;
	std::shared_ptr<SSAOMaterial>				m_pSSAOMaterial;
	std::shared_ptr<GaussianBlurMaterial>		m_pSSAOBlurVMaterial;
	std::shared_ptr<GaussianBlurMaterial>		m_pSSAOBlurHMaterial;
	std::shared_ptr<DeferredShadingMaterial>	m_pShadingMaterial;
	std::shared_ptr<ForwardMaterial>			m_pSkyBoxMaterial;
	std::shared_ptr<TemporalResolveMaterial>	m_pTemporalResolveMaterial;
	std::vector<std::shared_ptr<DOFMaterial>>	m_DOFMaterials;
	std::vector<std::shared_ptr<BloomMaterial>>	m_bloomDownsampleMaterials;
	std::vector<std::shared_ptr<BloomMaterial>>	m_bloomUpsampleMaterials;
	std::shared_ptr<GaussianBlurMaterial>		m_pBloomBlurVMaterial;
	std::shared_ptr<GaussianBlurMaterial>		m_pBloomBlurHMaterial;
	std::shared_ptr<CombineMaterial>			m_pCombineMaterial;
	std::shared_ptr<PostProcessingMaterial>		m_pPostProcessMaterial;
};