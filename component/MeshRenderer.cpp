#include "MeshRenderer.h"

std::shared_ptr<MeshRenderer> MeshRenderer::Create(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh)
{
	std::shared_ptr<MeshRenderer> pMeshRenderer = std::make_shared<MeshRenderer>();
	if (pMeshRenderer.get() && pMeshRenderer->Init(pMeshRenderer, pMesh))
		return pMeshRenderer;
	return nullptr;
}

std::shared_ptr<MeshRenderer> MeshRenderer::Create(const std::shared_ptr<MeshRenderer>& pSelf)
{
	std::shared_ptr<MeshRenderer> pMeshRenderer = std::make_shared<MeshRenderer>();
	if (pMeshRenderer.get() && pMeshRenderer->Init(pMeshRenderer, nullptr))
		return pMeshRenderer;
	return nullptr;
}

bool MeshRenderer::Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	m_pMesh = pMesh;
}

void MeshRenderer::Update(const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{

}

void MeshRenderer::LateUpdate(const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{

}