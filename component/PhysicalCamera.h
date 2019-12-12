#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../Maths/PyramidFrustum.h"
#include "../class/FrameBufferDiction.h"

class PhysicalCamera : public BaseComponent
{
	DECLARE_CLASS_RTTI(PhysicalCamera);

public:
	typedef struct _PhysicalCameraProps
	{
		//projection related variables
		float		aspect			= FrameBufferDiction::WINDOW_WIDTH / FrameBufferDiction::WINDOW_HEIGHT;
		float       filmWidth		= 0.035f;           // default standard film width 35mm
		float       focalLength		= 0.035f;			// default standard focal length 35mm
		float       focusDistance	= 1;				// default focus distance 10m
		float       fstop			= 3.5f;				// default fstop 3.5
		float       shutterSpeed	= 1 / 500.0f;		// default shutter speed 1/500s
		float       ISO				= 100.0f;			// default ISO 100
		float		farPlane		= 2000.0f;			// default far plane is 2000
	}PhysicalCameraProps;

	// Supplement properties are deduced by camera properties, which will update it when itself was updated
	typedef struct _PhysicalCameraSupplementProps
	{
		float		filmHeight;
		float		horizontalFOV_2;		// Half horizontal FOV in radius
		float		verticalFOV_2;			// Half vertical FOV in radius
		float		tangentHorizontalFOV_2;	// Tangent of half horizontal FOV
		float		tangentVerticalFOV_2;	// Tangent of half vertical FOV
		float		apertureDiameter;

		float		fixedNearPlane = 0.01f;	// Near plane is fixed, focal length & film width decide only fov
		float		fixedNearPlaneWidth;	// The size of near plane is fixed
		float		fixedNearPlaneHeight;	// The size of near plane is fixed
	}PhysicalCameraSupplementProps;

public:
	void Update() override;
	void OnPreRender() override;

	void SetJitterOffset(Vector2f jitterOffset);
	void SetFilmWidth(float filmWidth);
	void SetFocalLength(float focalLength);
	void SetFStop(float fstop);
	void SetShutterSpeed(float shutterSpeed);
	void SetISO(float ISO);
	void SetFarPlane(float farPlane);

	void SetCameraProps(const PhysicalCameraProps& props);

	const PhysicalCameraProps& GetCameraProps() const { return m_props; }
	const PhysicalCameraSupplementProps& GetCameraSupplementProps() const { return m_supplementProps; }
	//const float GetFovH() const { return m_fovH; }

	const Matrix4f GetVPMatrix() const { return m_vpMatrix; }
	const Vector3f GetCameraDir() const;
	PyramidFrustumf GetCameraFrustum() const { return m_frustum; }

	static std::shared_ptr<PhysicalCamera> Create(const PhysicalCameraProps& props);

protected:
	bool Init(const PhysicalCameraProps& props, const std::shared_ptr<PhysicalCamera>& pCamera);
	void UpdateViewMatrix();
	void UpdateProjMatrix();
	void UpdateCameraProps();
	void UpdateFOVRelatedProps();
	void UpdateCameraSupplementProps();

protected:
	PhysicalCameraProps				m_props;
	PhysicalCameraSupplementProps	m_supplementProps;

	PyramidFrustumf					m_frustum;

	Matrix4f						m_vpMatrix;
	bool							m_projDirty;
	bool							m_propDirty;

	Vector2f						m_jitterOffset;
};