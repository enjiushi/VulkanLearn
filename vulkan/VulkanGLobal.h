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
	void InitDescriptorSetLayout();
	void InitPipelineCache();
	void InitPipeline();
	void InitDescriptorPool();
	void InitDescriptorSet();
	void InitDrawCmdBuffers();
	void InitSemaphore();
	void EndSetup();

	void PrepareDrawCommandBuffer(const std::shared_ptr<PerFrameResource>& pPerFrameRes);
	void Draw();
	void Update();

	static void Free();

	void InitShaderModule(const char* vertShaderPath, const char* fragShaderPath);

public:
	static const uint32_t				WINDOW_WIDTH = 1024;
	static const uint32_t				WINDOW_HEIGHT = 768;

protected:
	std::shared_ptr<Instance>			m_pVulkanInstance;
	std::shared_ptr<PhysicalDevice>		m_pPhysicalDevice;
	std::shared_ptr<Device>				m_pDevice;

	std::shared_ptr<CommandPool>		m_pCommandPool;
	std::shared_ptr<CommandBuffer>		m_pPrePresentCmdBuffer;
	std::shared_ptr<CommandBuffer>		m_pPostPresentCmdBuffer;

	std::shared_ptr<DepthStencilBuffer>	m_pDSBuffer;

	std::shared_ptr<RenderPass>			m_pRenderPass;
	std::vector<std::shared_ptr<FrameBuffer>>m_framebuffers;

	std::shared_ptr<VertexBuffer>		m_pVertexBuffer;
	std::shared_ptr<IndexBuffer>		m_pIndexBuffer;
	std::shared_ptr<UniformBuffer>		m_pUniformBuffer;

	GlobalUniforms						m_globalUniforms;

	std::shared_ptr<DescriptorSetLayout>m_pDescriptorSetLayout;
	std::shared_ptr<PipelineLayout>		m_pPipelineLayout;

	std::shared_ptr<GraphicPipeline>	m_pPipeline;

	std::shared_ptr<DescriptorPool>		m_pDescriptorPool;
	std::shared_ptr<DescriptorSet>		m_pDescriptorSet;

	std::shared_ptr<ShaderModule>		m_pVertShader;
	std::shared_ptr<ShaderModule>		m_pFragShader;

	//std::vector<VkCommandBuffer>		m_drawCmdBuffers;
	std::vector<std::shared_ptr<CommandBuffer>>		m_drawCmdBuffers;

	std::vector<std::shared_ptr<PerFrameResource>> m_perFrameRes;

	std::shared_ptr<ThreadTaskQueue>	m_pThreadTaskQueue;
	std::mutex							m_updateMutex;

	float								m_roughness = 0.1;
#if defined(_WIN32)
	HINSTANCE							m_hPlatformInst;
	HWND								m_hWindow;
#endif
};