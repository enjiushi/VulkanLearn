#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include "CustomizedComputeMaterial.h"
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
class GBufferPlanetMaterial;
class Material;

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

	enum MaterialEnum
	{
		PBRGBuffer,
		PBRSkinnedGBuffer,
		PBRPlanetGBuffer,
		BackgroundMotion,
		MotionTileMax,
		MotionNeighborMax,
		Shadow,
		SkinnedShadow,
		SSAOSSR,
		SSAOBlurV,
		SSAOBlurH,
		DeferredShading,
		TemporalResolve,
		DepthOfField,
		BloomDownSample,
		BloomUpSample,
		Combine,
		PostProcess,
		SkyboxGen,
		ReflectionGen1,
		IrradianceGen1,
		MaterialEnumCount
	};

public:
	bool Init();

public:
	void SetRenderStateMask(RenderState renderState) { m_renderStateMask = (1 << renderState); }
	void SetRenderStateMask(uint32_t mask) { m_renderStateMask = mask; }
	void AddRenderStateMask(RenderState renderState) { m_renderStateMask |= (1 << renderState); }
	uint32_t GetRenderStateMask() const { return m_renderStateMask; }

	std::shared_ptr<MaterialInstance> AcquirePBRMaterialInstance() const;
	std::shared_ptr<MaterialInstance> AcquirePBRSkinnedMaterialInstance() const;
	std::shared_ptr<MaterialInstance> AcquirePBRPlanetMaterialInstance() const;
	std::shared_ptr<MaterialInstance> AcquireShadowMaterialInstance() const;
	std::shared_ptr<MaterialInstance> AcquireSkinnedShadowMaterialInstance() const;

	void SyncMaterialData();
	void Draw(const std::shared_ptr<CommandBuffer>& pDrawCmdBuffer, uint32_t pingpong);

	void OnFrameBegin();
	void OnFrameEnd();

	std::shared_ptr<Material>	GetMaterial(MaterialEnum materialEnum, uint32_t index = 0) const { return m_materials[materialEnum].GetMaterial(index); }

protected:
	// Since there could be some mutants of the same material class
	// We encapsulate these one or more materials into "MaterialSet"
	typedef struct _MaterialSet
	{
		std::vector<std::shared_ptr<Material>>	materialSet;
		std::shared_ptr<Material> GetMaterial(uint32_t index = 0) const { return materialSet[index]; }
	}MaterialSet;

	std::vector<MaterialSet>	m_materials;
	uint32_t					m_renderStateMask;
};