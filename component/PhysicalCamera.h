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
		double		aspect			= FrameBufferDiction::WINDOW_WIDTH / FrameBufferDiction::WINDOW_HEIGHT;
		double      filmWidth		= 0.035;           // default standard film width 35mm
		double      focalLength		= 0.035;			// default standard focal length 35mm
		double      focusDistance	= 1;				// default focus distance 10m
		double      fstop			= 3.5;				// default fstop 3.5
		double      shutterSpeed	= 1 / 500.0;		// default shutter speed 1/500s
		double      ISO				= 100.0;			// default ISO 100
		double		farPlane		= 2000.0;			// default far plane is 2000
	}PhysicalCameraProps;

	// Supplement properties are deduced by camera properties, which will update it when itself was updated
	typedef struct _PhysicalCameraSupplementProps
	{
		double		filmHeight;
		double		horizontalFOV_2;		// Half horizontal FOV in radius
		double		verticalFOV_2;			// Half vertical FOV in radius
		double		tangentHorizontalFOV_2;	// Tangent of half horizontal FOV
		double		tangentVerticalFOV_2;	// Tangent of half vertical FOV
		double		apertureDiameter;

		double		fixedNearPlane = 0.01;	// Near plane is fixed, focal length & film width decide only fov
		double		fixedNearPlaneWidth;	// The size of near plane is fixed
		double		fixedNearPlaneHeight;	// The size of near plane is fixed
	}PhysicalCameraSupplementProps;

public:
	void Update() override;
	void OnPreRender() override;

	void SetJitterOffset(Vector2d jitterOffset);
	void SetFilmWidth(double filmWidth);
	void SetFocalLength(double focalLength);
	void SetFStop(double fstop);
	void SetShutterSpeed(double shutterSpeed);
	void SetISO(double ISO);
	void SetFarPlane(double farPlane);

	void SetCameraProps(const PhysicalCameraProps& props);

	const PhysicalCameraProps& GetCameraProps() const { return m_props; }
	const PhysicalCameraSupplementProps& GetCameraSupplementProps() const { return m_supplementProps; }
	//const float GetFovH() const { return m_fovH; }

	const Matrix4d GetVPMatrix() const { return m_vpMatrix; }
	const Vector3d GetCameraDir() const;
	PyramidFrustumd GetCameraFrustum() const { return m_frustum; }

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

	PyramidFrustumd					m_frustum;

	Matrix4d						m_vpMatrix;
	bool							m_projDirty;
	bool							m_propDirty;

	Vector2d						m_jitterOffset;
};