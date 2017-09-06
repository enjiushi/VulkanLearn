#pragma once
#include "../common/Singleton.h"
#include "vulkan.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "SwapChain.h"
#include <vector>
#include <memory>
#include "DeviceMemoryManager.h"
#include "Buffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "GlobalDeviceObjects.h"
#include "DepthStencilBuffer.h"
#include "SwapChainImage.h"
#include "RenderPass.h"
#include "DescriptorSetLayout.h"
#include "PipelineLayout.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "ShaderModule.h"
#include "GraphicPipeline.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Semaphore.h"
#include "PerFrameResource.h"
#include "../thread/ThreadTaskQueue.hpp"
#include "../Base/BaseObject.h"
#include "../component/Camera.h"
#include "../component/Character.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "../component/Mesh.h"
#include "../component/MeshRenderer.h"
#include "../component/Material.h"
#include "../component/MaterialInstance.h"

typedef struct _GlobalUniforms
{
	float	model[16];
	float	view[16];
	float	projection[16];
	float	vulkanNDC[16];
	float	mvp[16];
	float   camPos[3];
	float	roughness;
}GlobalUniforms;

class VulkanGlobal : public Singleton<VulkanGlobal>
{
	const static uint32_t OffScreenSize = 512;

public:
#if defined(_WIN32)
	void Init(HINSTANCE hInstance, WNDPROC wndproc);
#endif
	void InitVulkanInstance();
	void InitPhysicalDevice(HINSTANCE hInstance, HWND hWnd);
	void InitVulkanDevice();
	void InitQueue();
#if defined(_WIN32)
	void SetupWindow(HINSTANCE hInstance, WNDPROC wndproc);
	void HandleMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
	void InitSurface();
	void InitSwapchain();

	void InitCommandPool();
	void InitSetupCommandBuffer();
	void InitMemoryMgr();
	void InitSwapchainImgs();
	void InitDepthStencil();
	void InitRenderpass();
	void InitFrameBuffer();
	void InitVertices();
	void InitUniforms();
	void InitIrradianceMap();
	void InitPrefilterEnvMap();
	void InitBRDFlutMap();
	void InitDescriptorSetLayout();
	void InitPipelineCache();
	void InitPipeline();
	void InitDescriptorPool();
	void InitDescriptorSet();
	void InitDrawCmdBuffers();
	void InitSemaphore();
	void InitMaterials();
	void EndSetup();

	void UpdateUniforms(uint32_t frameIndex, const std::shared_ptr<Camera>& pCamera);
	void Draw();
	void Update();

	void InitShaderModule();

public:
	static const uint32_t				WINDOW_WIDTH = 1024;
	static const uint32_t				WINDOW_HEIGHT = 768;

public:
	std::shared_ptr<Instance>			m_pVulkanInstance;
	std::shared_ptr<PhysicalDevice>		m_pPhysicalDevice;
	std::shared_ptr<Device>				m_pDevice;

	std::shared_ptr<CommandPool>		m_pCommandPool;
	std::shared_ptr<CommandBuffer>		m_pPrePresentCmdBuffer;
	std::shared_ptr<CommandBuffer>		m_pPostPresentCmdBuffer;

	std::shared_ptr<DepthStencilBuffer>	m_pDSBuffer;

	std::shared_ptr<FrameBuffer>		m_pEnvFrameBuffer;
	std::vector<std::shared_ptr<FrameBuffer>>m_offscreenFrameBuffers;


	std::shared_ptr<Mesh>				m_pGunMesh;
	std::shared_ptr<Mesh>				m_pCubeMesh;
	std::shared_ptr<Mesh>				m_pQuadMesh;

	std::shared_ptr<UniformBuffer>		m_pUniformBuffer;

	std::shared_ptr<DescriptorSetLayout>m_pSkyBoxDSLayout;

	std::shared_ptr<PipelineLayout>		m_pSkyBoxPLayout;

	std::shared_ptr<GraphicPipeline>	m_pSkyBoxPipeline;
	std::shared_ptr<GraphicPipeline>	m_pOffScreenIrradiancePipeline;
	std::shared_ptr<GraphicPipeline>	m_pOffScreenPrefilterEnvPipeline;
	std::shared_ptr<GraphicPipeline>	m_pOffScreenBRDFLutPipeline;
	std::shared_ptr<GraphicPipeline>	m_pSimplePipeline;

	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;
	std::shared_ptr<DescriptorSet>		m_pSkyBoxDS;
	std::shared_ptr<DescriptorSet>		m_pSimpleDS;

	std::shared_ptr<ShaderModule>		m_pSkyBoxVS;
	std::shared_ptr<ShaderModule>		m_pSkyBoxFS;

	std::shared_ptr<ShaderModule>		m_pSimpleVS;
	std::shared_ptr<ShaderModule>		m_pSimpleFS;

	std::shared_ptr<ShaderModule>		m_pIrradianceFS;
	std::shared_ptr<ShaderModule>		m_pPrefilterEnvFS;
	std::shared_ptr<ShaderModule>		m_pBRDFLutVS;
	std::shared_ptr<ShaderModule>		m_pBRDFLutFS;

	//std::vector<VkCommandBuffer>		m_drawCmdBuffers;
	std::vector<std::shared_ptr<CommandBuffer>>		m_drawCmdBuffers;

	std::vector<std::shared_ptr<PerFrameResource>> m_perFrameRes;

	std::shared_ptr<ThreadTaskQueue>	m_pThreadTaskQueue;
	std::mutex							m_updateMutex;

	std::shared_ptr<BaseObject>			m_pCameraObj;
	std::shared_ptr<Camera>				m_pCameraComp;
	std::shared_ptr<BaseObject>			m_pOffScreenCamObj;
	std::shared_ptr<Camera>				m_pOffScreenCamComp;
	std::shared_ptr<Character>			m_pCharacter;
	uint32_t							m_moveFlag = 0;

	std::shared_ptr<Texture2D>			m_pAlbedo;
	std::shared_ptr<Texture2D>			m_pRoughness;
	std::shared_ptr<Texture2D>			m_pNormal;
	std::shared_ptr<Texture2D>			m_pAmbientOcclusion;
	std::shared_ptr<Texture2D>			m_pMetalic;
	std::shared_ptr<TextureCube>		m_pSkyBoxTex;
	std::shared_ptr<Texture2D>			m_pSimpleTex;
	std::shared_ptr<TextureCube>		m_pIrradianceTex;
	std::shared_ptr<TextureCube>		m_pPrefilterEnvTex;
	std::shared_ptr<Texture2D>			m_pBRDFLut;

	std::shared_ptr<BaseObject>			m_pGunObject;
	std::shared_ptr<MeshRenderer>		m_pGunMeshRenderer;
	std::shared_ptr<Material>			m_pGunMaterial;
	std::shared_ptr<MaterialInstance>	m_pGunMaterialInstance;

	std::shared_ptr<BaseObject>			m_pSkyBoxObject;
	std::shared_ptr<MeshRenderer>		m_pSkyBoxMeshRenderer;
	std::shared_ptr<Material>			m_pSkyBoxMaterial;
	std::shared_ptr<MaterialInstance>	m_pSkyBoxMaterialInstance;

	std::shared_ptr<BaseObject>			m_pRootObject;

	float								m_roughness = 0.1;
#if defined(_WIN32)
	HINSTANCE							m_hPlatformInst;
	HWND								m_hWindow;
#endif
	GlobalUniforms						m_globalUniforms;
};