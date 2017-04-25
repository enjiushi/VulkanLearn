#pragma once
#include "../common/Singleton.h"
#include "vulkan.h"
#include "VulkanInstance.h"
#include "PhysicalDevice.h"
#include "VulkanDevice.h"
#include "SwapChain.h"
#include <vector>
#include <memory>

typedef struct _swapchainImg
{
	std::vector<VkImage>				images;
	std::vector<VkImageView>			views;
}SwapchainImg;

typedef struct _depthStencil
{
	VkFormat							format;
	VkDeviceMemory						memory;
	VkImage								image;
	VkImageView							view;
}DepthStencil;

typedef struct _mvp
{
	float	model[16];
	float	view[16];
	float	projection[16];
	float	vulkanNDC[16];
	float	mvp[16];

	VkDescriptorBufferInfo mvpDescriptor;
}MVP;


typedef struct _buffer
{
	_buffer() { info = {}; info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; }
	VkDeviceMemory						memory;
	VkBuffer							buffer;
	VkMemoryRequirements				reqs;
	VkBufferCreateInfo					info;
	VkVertexInputBindingDescription		bindingDesc;
	uint32_t							count;
	std::vector<VkVertexInputAttributeDescription>	attribDesc;
}Buffer;

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

	void Draw();
	void Update();

	VkShaderModule InitShaderModule(const char* shaderPath);

public:
	static const uint32_t				WINDOW_WIDTH = 1024;
	static const uint32_t				WINDOW_HEIGHT = 768;

protected:
	std::shared_ptr<VulkanInstance>		m_vulkanInst;
	std::shared_ptr<PhysicalDevice>		m_physicalDevice;
	std::shared_ptr<VulkanDevice>		m_pDevice;
	std::shared_ptr<SwapChain>			m_pSwapchain;

	VkQueue								m_queue;

	VkCommandPool						m_commandPool;
	VkCommandBuffer						m_setupCommandBuffer;

	SwapchainImg						m_swapchainImg;
	DepthStencil						m_depthStencil;

	VkRenderPass						m_renderpass;
	std::vector<VkFramebuffer>			m_framebuffers;

	Buffer								m_vertexBuffer;
	Buffer								m_indexBuffer;
	Buffer								m_uniformBuffer;

	MVP									m_mvp;

	std::vector<VkDescriptorSetLayoutBinding> m_dsLayoutBinding;
	VkDescriptorSetLayout				m_descriptorSetLayout;
	VkPipelineLayout					m_pipelineLayout;

	VkPipeline							m_pipeline;
	VkPipelineCache						m_pipelineCache;

	VkDescriptorPool					m_descriptorPool;
	VkDescriptorSet						m_descriptorSet;

	std::vector<VkCommandBuffer>		m_drawCmdBuffers;
	VkCommandBuffer						m_prePresentCmdBuffer;
	VkCommandBuffer						m_postPresentCmdBuffer;

	VkSemaphore							m_swapchainAcquireDone;
	VkSemaphore							m_renderDone;

	uint32_t							m_currentBufferIndex = 0;
#if defined(_WIN32)
	HINSTANCE							m_hPlatformInst;
	HWND								m_hWindow;
#endif
};