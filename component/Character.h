#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"

class Camera;

enum CharMoveDir
{
	Forward = 1,
	Backward = 2,
	Leftward = 4,
	Rightward = 8
};

typedef struct _CharacterVariable
{
	float moveSpeed;
}CharacterVariable;

class Character : public BaseComponent
{
public:
	Character() {}
	~Character() {}

	void Move(uint32_t dir, float delta);

	void OnRotateStart(const Vector2f& v);

	void OnRotate(const Vector2f& v, bool started);
	void OnRotateEnd(const Vector2f& v);

	//input para v needs to be the ratio compared to camera size
	//this function should be used by those "OnRotate*" functions
	//but if user of this class decided to handle rotate logic himself, call this function directly
	void Rotate(const Vector2f& v);

	void SetCharacterVariable(const CharacterVariable& var) { m_charVars = var; }
	CharacterVariable GetCharacterVariable() const { return m_charVars; }

	void SetCamera(const std::shared_ptr<Camera>& pCamera) { m_camera = pCamera; }

protected:
	void Move(const Vector3f& v, float delta);

protected:
	Matrix3f			m_rotationStartMatrix;
	CharacterVariable	m_charVars;
	bool				m_rotationStarted;
	Vector2f			m_rotationStartPos;
	float				m_startTargetToH;
	Vector3f			m_lastTarget;
	std::shared_ptr<Camera>	m_camera;
};