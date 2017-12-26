#include "../Base/Base.h"
#include "../Base/BaseObject.h"
#include "../Base/BaseComponent.h"
#include "../common/Singleton.h"

class Camera;
class Mesh;
class MeshRenderer;
class Material;
class MaterialInstance;

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
	std::shared_ptr<Material> GetMaterial0() const { return m_pMaterial0; }
	std::shared_ptr<MaterialInstance> GetMaterialInstance0() const { return m_pMaterialInstance0; }

public:
	static std::shared_ptr<Mesh> GenerateBoxMesh();
	static std::shared_ptr<Mesh> GenerateQuadMesh();
	static std::shared_ptr<Material> GenerateIrradianceGenMaterial(const std::shared_ptr<Mesh>& pMesh);
	static std::shared_ptr<Material> GeneratePrefilterEnvGenMaterial(const std::shared_ptr<Mesh>& pMesh);
	static std::shared_ptr<Material> GenerateBRDFLUTGenMaterial(const std::shared_ptr<Mesh>& pMesh);
	static std::shared_ptr<BaseObject> GenerateIBLGenOffScreenCamera(uint32_t screenSize);

protected:
	std::shared_ptr<BaseObject>			m_pRootObj;
	std::shared_ptr<BaseObject>			m_pCameraObj;

	std::shared_ptr<Mesh>				m_pMesh0;
	std::shared_ptr<MeshRenderer>		m_pMeshRenderer0;

	std::shared_ptr<MaterialInstance>	m_pMaterialInstance0;
	std::shared_ptr<Material>			m_pMaterial0;
};