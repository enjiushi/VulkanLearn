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

private:
	typedef std::pair<Vector3f, uint32_t> VertexIndex;
	static void GenerateTriangles(uint32_t level, const VertexIndex& a, const VertexIndex& b, const VertexIndex& c, std::vector<Vector3f>& vertices, std::vector<uint32_t>& indices);

public:
	template<typename T>
	static void SubDivideTriangle(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& c, Vector3<T>& A, Vector3<T>& B, Vector3<T>& C)
	{
		A = c;
		A += b;
		A *= 0.5f;

		B = c;
		B += a;
		B *= 0.5f;

		C = b;
		C += a;
		C *= 0.5f;
	}

	static std::shared_ptr<Mesh> GenerateTriangleMesh(uint32_t level = 0);
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