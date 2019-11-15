#pragma once

#include "../Base/Base.h"
#include "../Base/BaseObject.h"
#include "../Base/BaseComponent.h"
#include "../common/Singleton.h"

class Camera;
class Mesh;
class MeshRenderer;
class Material;
class MaterialInstance;
class ForwardMaterial;

class SceneGenerator : public Singleton<SceneGenerator>
{
public:
	void PurgeExcistSceneData();

public:
	void GenerateIrradianceGenScene();
	void GeneratePrefilterEnvGenScene();
	void GenerateBRDFLUTGenScene();

public:
	std::shared_ptr<BaseObject> GetRootObject() const { return m_pRootObj; }
	std::shared_ptr<BaseObject> GetCameraObject() const { return m_pCameraObj; }
	std::shared_ptr<ForwardMaterial> GetMaterial0() const { return m_pMaterial0; }
	std::shared_ptr<MaterialInstance> GetMaterialInstance0() const { return m_pMaterialInstance0; }
	std::shared_ptr<Mesh> GetMesh0() const { return m_pMesh0; }
	std::shared_ptr<MeshRenderer> GetMeshRenderer0() const { return m_pMeshRenderer0; }

public:
	static std::shared_ptr<Mesh> GenerateTriangleMesh();
	static std::shared_ptr<Mesh> GenerateBoxMesh();
	static std::shared_ptr<Mesh> GenerateQuadMesh();
	static std::shared_ptr<Mesh> GeneratePBRQuadMesh();
	static std::shared_ptr<Mesh> GeneratePBRBoxMesh();
	static std::shared_ptr<Mesh> GenPBRIcosahedronMesh();
	static std::shared_ptr<ForwardMaterial> GenerateIrradianceGenMaterial(const std::shared_ptr<Mesh>& pMesh);
	static std::shared_ptr<ForwardMaterial> GeneratePrefilterEnvGenMaterial(const std::shared_ptr<Mesh>& pMesh);
	static std::shared_ptr<ForwardMaterial> GenerateBRDFLUTGenMaterial();
	static std::shared_ptr<BaseObject> GenerateIBLGenOffScreenCamera(uint32_t screenSize);

protected:
	std::shared_ptr<BaseObject>			m_pRootObj;
	std::shared_ptr<BaseObject>			m_pCameraObj;

	std::shared_ptr<Mesh>				m_pMesh0;
	std::shared_ptr<MeshRenderer>		m_pMeshRenderer0;

	std::shared_ptr<MaterialInstance>	m_pMaterialInstance0;
	std::shared_ptr<ForwardMaterial>	m_pMaterial0;
};