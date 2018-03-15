#include <mutex>
#include "Material.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/PhysicalDevice.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/SharedVertexBuffer.h"
#include "../vulkan/SharedIndexBuffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/StagingBufferManager.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GraphicPipeline.h"
#include "../vulkan/DescriptorSetLayout.h"
#include "../vulkan/PipelineLayout.h"
#include "../vulkan/ShaderModule.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/DescriptorPool.h"
#include "../class/MaterialInstance.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "../class/UniformData.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Image.h"
#include "../vulkan/SharedIndirectBuffer.h"
#include "RenderWorkManager.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../class/PerMaterialIndirectUniforms.h"
#include "../vulkan/Image.h"
#include "GlobalTextures.h"
#include "../vulkan/Image.h"
#include "../vulkan/Texture2DArray.h"
#include "../vulkan/TextureCube.h"
#include "../vulkan/Texture2D.h"
#include "../class/PerMaterialUniforms.h"

std::shared_ptr<Material> Material::CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo)
{
	std::shared_ptr<Material> pMaterial = std::make_shared<Material>();

	VkGraphicsPipelineCreateInfo createInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> blendStatesInfo =
	{
		{
			VK_TRUE,							// blend enabled

			VK_BLEND_FACTOR_SRC_ALPHA,			// src color blend factor
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,// dst color blend factor
			VK_BLEND_OP_ADD,					// color blend op

			VK_BLEND_FACTOR_ONE,				// src alpha blend factor
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,// dst alpha blend factor
			VK_BLEND_OP_ADD,					// alpha blend factor

			0xf,								// color mask
		},
	};

	VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
	blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendCreateInfo.logicOpEnable = VK_FALSE;
	blendCreateInfo.attachmentCount = 1;
	blendCreateInfo.pAttachments = blendStatesInfo.data();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	VkPipelineInputAssemblyStateCreateInfo assemblyCreateInfo = {};
	assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineMultisampleStateCreateInfo multiSampleCreateInfo = {};
	multiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pScissors = nullptr;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pViewports = nullptr;

	std::vector<VkDynamicState>	 dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStatesCreateInfo = {};
	dynamicStatesCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStatesCreateInfo.dynamicStateCount = dynamicStates.size();
	dynamicStatesCreateInfo.pDynamicStates = dynamicStates.data();

	std::vector<VkVertexInputBindingDescription> vertexBindingsInfo(simpleMaterialInfo.vertexBindingsInfo.size());
	for (uint32_t i = 0; i < simpleMaterialInfo.vertexBindingsInfo.size(); i++)
		vertexBindingsInfo[i] = simpleMaterialInfo.vertexBindingsInfo[i];

	std::vector<VkVertexInputAttributeDescription> vertexAttributesInfo(simpleMaterialInfo.vertexAttributesInfo.size());
	for (uint32_t i = 0; i < simpleMaterialInfo.vertexAttributesInfo.size(); i++)
		vertexAttributesInfo[i] = simpleMaterialInfo.vertexAttributesInfo[i];

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = vertexBindingsInfo.size();
	vertexInputCreateInfo.pVertexBindingDescriptions = vertexBindingsInfo.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexAttributesInfo.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributesInfo.data();

	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pColorBlendState = &blendCreateInfo;
	createInfo.pDepthStencilState = &depthStencilCreateInfo;
	createInfo.pInputAssemblyState = &assemblyCreateInfo;
	createInfo.pMultisampleState = &multiSampleCreateInfo;
	createInfo.pRasterizationState = &rasterizerCreateInfo;
	createInfo.pViewportState = &viewportStateCreateInfo;
	createInfo.pDynamicState = &dynamicStatesCreateInfo;
	createInfo.pVertexInputState = &vertexInputCreateInfo;
	createInfo.renderPass = simpleMaterialInfo.pRenderPass->GetDeviceHandle();

	if (pMaterial.get() && pMaterial->Init(pMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, simpleMaterialInfo.maxMaterialInstance, simpleMaterialInfo.materialVariableLayout, simpleMaterialInfo.vertexFormat))
		return pMaterial;
	return nullptr;
}

bool Material::Init
(
	const std::shared_ptr<Material>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPass>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	uint32_t maxMaterialInstance,
	const std::vector<UniformVarList>& materialVariableLayout,
	uint32_t vertexFormat
)
{
	if (!SelfRefBase<Material>::Init(pSelf))
		return false;

	std::vector<std::vector<UniformVarList>> _materialVariableLayout;
	_materialVariableLayout.resize(UniformDataStorage::UniformTypeCount);

	// Add per material indirect index uniform layout
	m_pPerMaterialIndirectUniforms = PerMaterialIndirectUniforms::Create();
	_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable].push_back(m_pPerMaterialIndirectUniforms->PrepareUniformVarList()[0]);

	// Add material variable layout
	_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable].insert(_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable].end(),
		materialVariableLayout.begin(), materialVariableLayout.end());

	// Force per object material variable to be shader storage buffer
	for (auto& var : _materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable])
	{
		if (var.type == DynamicUniformBuffer)
			var.type = DynamicShaderStorageBuffer;
	}

	// Obtain predefined uniform layout from global uniforms, perframe uniforms and perobject uniforms
	std::vector<std::vector<UniformVarList>> predefinedUniformLayout = UniformData::GetInstance()->GenerateUniformVarLayout();

	// And insert them into beginning of material variable layout
	for (uint32_t i = 0; i < UniformDataStorage::PerObjectMaterialVariable; i++)
	{
		_materialVariableLayout[i] = predefinedUniformLayout[i];
	}

	m_materialVariableLayout = _materialVariableLayout;

	// Build vulkan layout bindings
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> layoutBindings;
	for (auto & variables : _materialVariableLayout)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto & var : variables)
		{ 
			switch (var.type)
			{
			case DynamicUniformBuffer:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
					1,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
				});

				break;
			case DynamicShaderStorageBuffer:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
					1,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
				});

				break;
			case CombinedSampler:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
				});
				break;
			case InputAttachment:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
					});
				break;
			default:
				ASSERTION(false);
				break;
			}
		}
		m_descriptorSetLayouts.push_back(DescriptorSetLayout::Create(GetDevice(), bindings));
		layoutBindings.push_back(bindings);
	}

	// Create pipeline layout
	m_pPipelineLayout = PipelineLayout::Create(GetDevice(), m_descriptorSetLayouts);

	// Init shaders
	std::vector<std::shared_ptr<ShaderModule>> shaders;
	
	for (uint32_t i = 0; i < (uint32_t)ShaderModule::ShaderTypeCount; i++)
	{
		if (shaderPaths[i] != L"")
			shaders.push_back(ShaderModule::Create(GetDevice(), shaderPaths[i], (ShaderModule::ShaderType)i, "main"));	//FIXME: hard-coded main
	}

	// Create pipeline
	m_pPipeline = GraphicPipeline::Create(GetDevice(), pipelineCreateInfo, shaders, pRenderPass, m_pPipelineLayout);

	// Prepare descriptor pool size according to resources used by this material
	std::vector<uint32_t> counts(VK_DESCRIPTOR_TYPE_RANGE_SIZE);
	for (auto & bindings : layoutBindings)
	{
		for (auto & binding : bindings)
		{
			counts[binding.descriptorType]++;
		}
	}

	std::vector<VkDescriptorPoolSize> descPoolSize;
	for (uint32_t i = 0; i < counts.size(); i++)
	{
		if (counts[i] != 0)
			descPoolSize.push_back({ (VkDescriptorType)i, maxMaterialInstance * counts[i] });
	}

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pPoolSizes = descPoolSize.data();
	descPoolInfo.poolSizeCount = descPoolSize.size();
	descPoolInfo.maxSets = maxMaterialInstance * UniformDataStorage::UniformTypeCount;

	m_pDescriptorPool = DescriptorPool::Create(GetDevice(), descPoolInfo);

	m_maxMaterialInstance = maxMaterialInstance;

	// Per material uniform container shares the same size as material variable layout
	// However, not all of them is filled since some of them are textures or something other than uniforms
	m_perMaterialUniforms.resize(m_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable].size() - 1);	// Last one is per material indirect uniform
	for (uint32_t i = 0; i < m_perMaterialUniforms.size(); i++)
	{
		auto variable = m_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable][i + 1];
		// Force DynamicUniformBuffer to DynamicShaderStorageBuffer
		// Since for per object material variable, uniform buffer isn't big enough
		if (variable.type == DynamicUniformBuffer || variable.type == DynamicShaderStorageBuffer)
		{
			uint32_t size = GetByteSize(m_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable][i + 1].vars);
			m_perMaterialUniforms[i] = PerMaterialUniforms::Create(size);
		}
		else
			ASSERTION(false);	// Not support self-defined texture here, all textures go through global texture array
	}

	// Allocate descriptor sets according to layouts
	for (auto & layout : m_descriptorSetLayouts)
		m_descriptorSets.push_back(m_pDescriptorPool->AllocateDescriptorSet(layout));

	UniformData::GetInstance()->SetupDescriptorSet(m_descriptorSets[UniformDataStorage::GlobalVariable], UniformDataStorage::GlobalVariable);
	UniformData::GetInstance()->SetupDescriptorSet(m_descriptorSets[UniformDataStorage::PerFrameVariable], UniformDataStorage::PerFrameVariable);
	UniformData::GetInstance()->SetupDescriptorSet(m_descriptorSets[UniformDataStorage::PerObjectVariable], UniformDataStorage::PerObjectVariable);

	// Bind per material variable buffer
	for (uint32_t i = 0; i < m_perMaterialUniforms.size(); i++)
	{
		if (m_perMaterialUniforms[i] != nullptr)
		{
			m_perMaterialUniforms[i]->SetupDescriptorSet(m_descriptorSets[UniformDataStorage::PerObjectMaterialVariable], i + 1);
			m_frameOffsets.push_back(m_perMaterialUniforms[i]->GetFrameOffset());
		}
	}

	// Bind per material indirect uniform
	m_pPerMaterialIndirectUniforms->SetupDescriptorSet(m_descriptorSets[UniformDataStorage::PerObjectMaterialVariable]);

	m_pIndirectBuffer = SharedIndirectBuffer::Create(GetDevice(), sizeof(VkDrawIndirectCommand) * MAX_INDIRECT_COUNT);

	m_vertexFormat = vertexFormat;

	return true;
}

// This function follows rule of std430
// Could be bugs in it
uint32_t Material::GetByteSize(std::vector<UniformVar>& UBOLayout)
{
	uint32_t offset = 0;
	uint32_t unitCount = 0;
	uint32_t maxAlignUnit = 1;		// Result of max align size, obtained by iterate over the whole layout
	for (auto & var : UBOLayout)
	{
		switch (var.type)
		{
		case OneUnit:
			unitCount += 1;
			if (maxAlignUnit < 1)
				maxAlignUnit = 1;
			break;
		case Vec2Unit:
			unitCount = (unitCount + 1) / 2 * 2;
			unitCount += 2;
			if (maxAlignUnit < 2)
				maxAlignUnit = 2;
			break;
		case Vec3Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 3;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		case Vec4Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		case Mat3Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4 * 3;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		case Mat4Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4 * 4;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		default:
			ASSERTION(false);
			break;
		}

		var.offset = offset;
		offset = unitCount * 4;
	}

	// I need to comment here, as I've encountered a wired bug that I just cannot simple get per material uniform data right if object count exceeds over 1
	// After some investigation, I've located this problem:
	// I take alignment into consideration between material variables, however, I didn't take it into consideration between 2 material objects
	// Let's take an example, say a material layout is : vec4, vec2, float, float, float
	// After running this function, offset is: 0-vec4, 4-vec2, 6-float, 7-float, 8-float
	// Things seem okay, but when I have 2 objects with same material, problem occurs:
	// 2 sets of material variable data is not aligned! Since the whole size of a set is 9 unit, and biggest variable is 4 unit!
	// So I have to add padding space to the end of each material variable set, by 3 unit
	// Then, each material variable set is 12 unit big, and is perfectly aligned with each other
	// After doing this, material data of the second object turns back to okay
	uint32_t tailUnitCount = unitCount % maxAlignUnit;
	if (tailUnitCount > 0)
		unitCount = (unitCount / maxAlignUnit + 1) * maxAlignUnit;

	return unitCount * 4;
}

std::shared_ptr<MaterialInstance> Material::CreateMaterialInstance()
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = std::make_shared<MaterialInstance>();

	if (pMaterialInstance.get() && pMaterialInstance->Init(pMaterialInstance))
	{
		for (uint32_t i = 0; i < m_perMaterialUniforms.size(); i++)
			if (m_perMaterialUniforms[i] != nullptr)
			{
				// Even if there could be more than one per material uniform buffer
				// We still mark only one buffer chunk index, since their value should be exactly the same
				pMaterialInstance->m_materialBufferChunkIndex = m_perMaterialUniforms[i]->AllocatePerObjectChunk();
			}

		pMaterialInstance->m_pMaterial = GetSelfSharedPtr();

		m_generatedInstances.push_back(pMaterialInstance);

		return pMaterialInstance;
	}

	return nullptr;
}

uint32_t Material::GetUniformBufferSize(uint32_t bindingIndex) const
{
	if (m_perMaterialUniforms[bindingIndex] != nullptr)
		return m_perMaterialUniforms[bindingIndex]->GetBuffer()->GetBufferInfo().size;
	return 0;
}

std::vector<uint32_t> Material::GetFrameOffsets() const 
{ 
	std::vector<uint32_t> offsets = m_frameOffsets;
	for (auto & var : offsets)
		var *= FrameMgr()->FrameIndex();
	return offsets;
}

void Material::SyncBufferData()
{
	for (auto & var : m_perMaterialUniforms)
		if (var != nullptr)
			var->SyncBufferData();

	m_pPerMaterialIndirectUniforms->SyncBufferData();
}

void Material::BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	pCmdBuffer->BindPipeline(GetGraphicPipeline());
}

void Material::BindDescriptorSet(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::vector<uint32_t> offsets = UniformData::GetInstance()->GetFrameOffsets();

	// Insert per material indirect uniform frame offset
	offsets.push_back(m_pPerMaterialIndirectUniforms->GetFrameOffset() * FrameMgr()->FrameIndex());

	// Insert material offsets
	std::vector<uint32_t> materialOffsets = GetFrameOffsets();
	offsets.insert(offsets.end(), materialOffsets.begin(), materialOffsets.end());

	pCmdBuffer->BindDescriptorSets(GetPipelineLayout(), GetDescriptorSets(), offsets);
}

void Material::SetMaterialTexture(uint32_t index, const std::shared_ptr<Image>& pTexture)
{
	GetDescriptorSet(UniformDataStorage::PerObjectMaterialVariable)->UpdateImage(index + 1, pTexture);
}

void Material::BindMeshData(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	pCmdBuffer->BindVertexBuffers({ VertexAttribBufferMgr(m_vertexFormat)->GetBuffer() });
	pCmdBuffer->BindIndexBuffer(IndexBufferMgr()->GetBuffer(), VK_INDEX_TYPE_UINT32);
}

void Material::Draw()
{
	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPerFrameRes()->AllocateSecondaryCommandBuffer();

	std::vector<VkClearValue> clearValues =
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0 }
	};

	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = RenderWorkManager::GetInstance()->GetCurrentRenderPass()->GetDeviceHandle();
	inheritanceInfo.subpass = RenderWorkManager::GetInstance()->GetCurrentRenderPass()->GetCurrentSubpass();
	inheritanceInfo.framebuffer = RenderWorkManager::GetInstance()->GetCurrentFrameBuffer()->GetDeviceHandle();
	pDrawCmdBuffer->StartSecondaryRecording(inheritanceInfo);

	VkViewport viewport =
	{
		0, 0,
		RenderWorkManager::GetInstance()->GetCurrentFrameBuffer()->GetFramebufferInfo().width, RenderWorkManager::GetInstance()->GetCurrentFrameBuffer()->GetFramebufferInfo().height,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		RenderWorkManager::GetInstance()->GetCurrentFrameBuffer()->GetFramebufferInfo().width, RenderWorkManager::GetInstance()->GetCurrentFrameBuffer()->GetFramebufferInfo().height,
	};

	pDrawCmdBuffer->SetViewports({ GetGlobalVulkanStates()->GetViewport() });
	pDrawCmdBuffer->SetScissors({ GetGlobalVulkanStates()->GetScissorRect() });

	BindPipeline(pDrawCmdBuffer);
	BindDescriptorSet(pDrawCmdBuffer);
	BindMeshData(pDrawCmdBuffer);

	pDrawCmdBuffer->DrawIndexedIndirect(m_pIndirectBuffer, 0, m_indirectIndex);

	pDrawCmdBuffer->EndSecondaryRecording();

	RenderWorkManager::GetInstance()->GetCurrentRenderPass()->CacheSecondaryCommandBuffer(pDrawCmdBuffer);
}

void Material::InsertIntoRenderQueue(const VkDrawIndexedIndirectCommand& cmd, uint32_t perObjectIndex, uint32_t perMaterialIndex)
{
	m_pIndirectBuffer->SetIndirectCmd(m_indirectIndex, cmd);
	m_pPerMaterialIndirectUniforms->SetPerObjectIndex(m_indirectIndex, perObjectIndex);
	m_pPerMaterialIndirectUniforms->SetPerMaterialIndex(m_indirectIndex, perMaterialIndex);
	m_indirectIndex += 1;
}

void Material::OnFrameStart()
{
	m_indirectIndex = 0;
}

void Material::OnFrameEnd()
{

}

void Material::SetPerObjectIndex(uint32_t indirectIndex, uint32_t perObjectIndex)
{
	m_pPerMaterialIndirectUniforms->SetPerObjectIndex(indirectIndex, perObjectIndex);
}

uint32_t Material::GetPerObjectIndex(uint32_t indirectIndex) const
{
	return m_pPerMaterialIndirectUniforms->GetPerObjectIndex(indirectIndex);
}

void Material::SetPerMaterialIndex(uint32_t indirectIndex, uint32_t perMaterialIndex)
{
	m_pPerMaterialIndirectUniforms->SetPerMaterialIndex(indirectIndex, perMaterialIndex);
}

uint32_t Material::GetPerMaterialIndex(uint32_t indirectIndex) const
{
	return m_pPerMaterialIndirectUniforms->GetPerMaterialIndex(indirectIndex);
}