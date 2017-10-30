#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "GlobalUniforms.h"
#include "Material.h"

bool GlobalUniforms::Init(const std::shared_ptr<GlobalUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_globalVariables)))
		return false;

	// NDC space Y axis is reversed in Vulkan compared to OpenGL
	m_globalVariables.vulkanNDC.c[1].y = -1.0f;

	// NDC space z ranges from 0 to 1 in Vulkan compared to OpenGL's -1 to 1
	m_globalVariables.vulkanNDC.c[2].z = m_globalVariables.vulkanNDC.c[3].z = 0.5f;

	SetDirty();

	return true;
}

std::shared_ptr<GlobalUniforms> GlobalUniforms::Create()
{
	std::shared_ptr<GlobalUniforms> pGlobalUniforms = std::make_shared<GlobalUniforms>();
	if (pGlobalUniforms.get() && pGlobalUniforms->Init(pGlobalUniforms))
		return pGlobalUniforms;
	return nullptr;
}

void GlobalUniforms::SetProjectionMatrix(const Matrix4f& proj) 
{ 
	m_globalVariables.projectionMatrix = proj;
	SetDirty();
}

void GlobalUniforms::SetVulkanNDCMatrix(const Matrix4f& vndc) 
{ 
	m_globalVariables.vulkanNDC = vndc;
	SetDirty();
}

void GlobalUniforms::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(&m_globalVariables, FrameMgr()->FrameIndex() * sizeof(m_globalVariables), sizeof(m_globalVariables));
}

void GlobalUniforms::SetDirty()
{
	m_globalVariables.PN = m_globalVariables.vulkanNDC * m_globalVariables.projectionMatrix;
	UniformDataStorage::SetDirty();
}

UniformVarList GlobalUniforms::PrepareUniformVarList()
{
	return 
	{
		DynamicUniformBuffer,
		"GlobalUniforms",
		{
			{
				Mat4Unit,
				"ProjectionMatrix"
			},
			{
				Mat4Unit,
				"VulkanNDCMatrix"
			}
		}
	};
}

