#include "DeviceObjectBase.h"

class RenderPass : public DeviceObjectBase<RenderPass>
{
public:
	~RenderPass();

public:
	static std::shared_ptr<RenderPass> Create(const std::shared_ptr<Device>& pDevice, const VkRenderPassCreateInfo& renderPassInfo);

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<RenderPass>& pSelf, const VkRenderPassCreateInfo& renderPassInfo);

public:
	VkRenderPass GetDeviceHandle() const { return m_renderPass; }

protected:

	typedef struct _SubpassDef
	{
		std::vector<VkAttachmentReference>		m_colorAttachmentRefs;
		VkAttachmentReference					m_depthStencilAttachmentRef;
		std::vector<VkAttachmentReference>		m_inputAttachmentRefs;
		std::vector<uint32_t>					m_preservAttachmentRefs;
		std::vector<VkAttachmentReference>		m_resolveAttachmentRefs;
	}SubpassDef;

	bool									m_inited = false;
	VkRenderPass							m_renderPass;
	VkRenderPassCreateInfo					m_renderPassInfo;
	std::vector<VkAttachmentDescription>	m_attachmentDescList;
	std::vector<SubpassDef>					m_subpasses;
	std::vector<VkSubpassDescription>		m_subpassDescList;
	std::vector<VkSubpassDependency>		m_subpassDependencyList;
};