#pragma once
#include "../Base/BaseComponent.h"

class Mesh;

class MeshRenderer : public BaseComponent
{
public:
	static std::shared_ptr<MeshRenderer> Create(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh);
	static std::shared_ptr<MeshRenderer> Create(const std::shared_ptr<MeshRenderer>& pSelf);

public:
	void Update(const std::shared_ptr<PerFrameResource>& pPerFrameRes) override;
	void LateUpdate(const std::shared_ptr<PerFrameResource>& pPerFrameRes) override;

protected:
	bool Init(const std::shared_ptr<MeshRenderer>& pSelf, const std::shared_ptr<Mesh> pMesh);

protected:
	std::shared_ptr<Mesh> m_pMesh;
};