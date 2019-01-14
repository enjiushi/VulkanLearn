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
#include "../class/Mesh.h"
#include "../component/MeshRenderer.h"
#include "../class/Material.h"
#include "../class/MaterialInstance.h"
#include "ShaderStorageBuffer.h"
#include "Texture2DArray.h"
#include "../class/ForwardMaterial.h"
#include "../class/DeferredMaterial.h"
#include "../component/DirectionLight.h"
#include "../class/ShadowMapMaterial.h"
#include "../class/SSAOMaterial.h"
#include "../class/GaussianBlurMaterial.h"
#include "../class/BloomMaterial.h"
#include "../class/PostProcessingMaterial.h"
#include "../component/PhysicalCamera.h"

class VulkanGlobal : public Singleton<VulkanGlobal>
{
	const static uint32_t OffScreenSize = 512;

public:
#if defined(_WIN32)
	void InitVulkan(HINSTANCE hInstance, WNDPROC wndproc);
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
	void InitDescriptorSetLayout();
	void InitPipelineCache();
	void InitPipeline();
	void InitDescriptorPool();
	void InitDescriptorSet();
	void InitDrawCmdBuffers();
	void InitSemaphore();
	void InitMaterials();
	void InitScene();
	void EndSetup();

	void Draw();
	void Update();

	void InitShaderModule();

public:
	std::shared_ptr<Instance>			m_pVulkanInstance;
	std::shared_ptr<PhysicalDevice>		m_pPhysicalDevice;
	std::shared_ptr<Device>				m_pDevice;

	std::shared_ptr<CommandPool>		m_pCommandPool;
	std::shared_ptr<CommandBuffer>		m_pPrePresentCmdBuffer;
	std::shared_ptr<CommandBuffer>		m_pPostPresentCmdBuffer;

	std::shared_ptr<DepthStencilBuffer>	m_pDSBuffer;


	std::shared_ptr<Mesh>				m_pGunMesh;
	std::vector<std::shared_ptr<Mesh>>	m_innerBallMeshes;
	std::vector<std::shared_ptr<Mesh>>	m_bunnyMeshes;
	std::vector<std::shared_ptr<Mesh>>	m_dragonMeshes;
	std::shared_ptr<Mesh>				m_pSphereMesh;
	std::shared_ptr<Mesh>				m_pCubeMesh;
	std::shared_ptr<Mesh>				m_pQuadMesh;
	std::shared_ptr<Mesh>				m_pPBRBoxMesh;

	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;

	//std::vector<VkCommandBuffer>		m_drawCmdBuffers;
	std::vector<std::shared_ptr<CommandBuffer>>		m_drawCmdBuffers;

	std::vector<std::shared_ptr<PerFrameResource>> m_perFrameRes;

	std::shared_ptr<ThreadTaskQueue>	m_pThreadTaskQueue;
	std::mutex							m_updateMutex;

	std::shared_ptr<BaseObject>			m_pCameraObj;
	std::shared_ptr<PhysicalCamera>		m_pCameraComp;
	std::shared_ptr<Character>			m_pCharacter;

	std::shared_ptr<BaseObject>			m_pDirLightObj;
	std::shared_ptr<DirectionLight>		m_pDirLight;

	std::shared_ptr<Texture2D>			m_pAlbedoRoughness;
	std::shared_ptr<Texture2D>			m_pNormalAO;
	std::shared_ptr<Texture2D>			m_pMetalic;
	std::shared_ptr<TextureCube>		m_pSkyBoxTex;
	std::shared_ptr<Texture2D>			m_pSimpleTex;
	std::shared_ptr<TextureCube>		m_pIrradianceTex;
	std::shared_ptr<TextureCube>		m_pPrefilterEnvTex;
	std::shared_ptr<Texture2D>			m_pBRDFLut;

	std::shared_ptr<BaseObject>			m_pGunObject;
	std::shared_ptr<BaseObject>			m_pSphere0;
	std::shared_ptr<BaseObject>			m_pSphere1;
	std::shared_ptr<BaseObject>			m_pSphere2;
	std::vector<std::shared_ptr<BaseObject>> m_innerBallObjects;
	std::shared_ptr<BaseObject>			m_pInnerBall;
	std::vector<std::shared_ptr<BaseObject>> m_bunnyObjects;
	std::shared_ptr<BaseObject>			m_pBunny;
	std::vector<std::shared_ptr<BaseObject>> m_dragonObjects;
	std::shared_ptr<BaseObject>			m_pDragon;
	std::shared_ptr<BaseObject>			m_pBoxObject0;
	std::shared_ptr<BaseObject>			m_pBoxObject1;
	std::shared_ptr<BaseObject>			m_pBoxObject2;
	std::shared_ptr<MeshRenderer>		m_pGunMeshRenderer;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer0;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer1;
	std::shared_ptr<MeshRenderer>		m_pSphereRenderer2;
	std::vector<std::shared_ptr<MeshRenderer>>	m_innerBallRenderers;
	std::vector<std::shared_ptr<MeshRenderer>>	m_bunnyRenderers;
	std::vector<std::shared_ptr<MeshRenderer>>	m_dragonRenderers;
	std::shared_ptr<MeshRenderer>		m_pQuadRenderer;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer0;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer1;
	std::shared_ptr<MeshRenderer>		m_pBoxRenderer2;

	std::shared_ptr<MaterialInstance>	m_pGunMaterialInstance;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance0;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance1;
	std::shared_ptr<MaterialInstance>	m_pSphereMaterialInstance2;
	std::vector<std::shared_ptr<MaterialInstance>> m_innerBallMaterialInstances;
	std::vector<std::shared_ptr<MaterialInstance>> m_bunnyMaterialInstances;
	std::vector<std::shared_ptr<MaterialInstance>> m_dragonMaterialInstances;
	std::shared_ptr<MaterialInstance>	m_pQuadMaterialInstance;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance0;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance1;
	std::shared_ptr<MaterialInstance>	m_pBoxMaterialInstance2;

	std::shared_ptr<MaterialInstance>	m_pShadowMapMaterialInstance;

	std::shared_ptr<BaseObject>			m_pSkyBoxObject;
	std::shared_ptr<MeshRenderer>		m_pSkyBoxMeshRenderer;
	std::shared_ptr<ForwardMaterial>	m_pSkyBoxMaterial;
	std::shared_ptr<MaterialInstance>	m_pSkyBoxMaterialInstance;

	std::shared_ptr<BaseObject>			m_pQuadObject;

	std::shared_ptr<BaseObject>			m_pTestObject;
	std::shared_ptr<MeshRenderer>		m_pTestRenderer;
	std::shared_ptr<Material>			m_pTestMaterial;
	std::shared_ptr<MaterialInstance>	m_pTestMaterialInstance;

	std::shared_ptr<BaseObject>			m_pRootObject;

	std::vector<std::shared_ptr<CommandBuffer>> m_commandBufferList;

#if defined(_WIN32)
	HINSTANCE							m_hPlatformInst;
	HWND								m_hWindow;
#endif
};