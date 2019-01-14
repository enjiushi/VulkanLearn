#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../class/InputHub.h"

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
	float rotateSpeed;
}CharacterVariable;

class Character : public BaseComponent, public IInputListener
{
	DECLARE_CLASS_RTTI(Character);

public:
	static std::shared_ptr<Character> Create(const CharacterVariable& charVar);

	// For input listener
	void ProcessKey(KeyState keyState, uint8_t keyCode) override;
	void ProcessMouse(KeyState keyState, const Vector2f& mousePosition) override;
	void ProcessMouse(const Vector2f& mousePosition) override;

	void Move(uint32_t dir, float delta);

	void OnRotateStart(const Vector2f& v);

	void OnRotate(const Vector2f& v, bool started);
	void OnRotate(uint32_t dir, float delta);

	void OnRotateEnd(const Vector2f& v);

	//input para v needs to be the ratio compared to camera size
	//this function should be used by those "OnRotate*" functions
	//but if user of this class decided to handle rotate logic himself, call this function directly
	void Rotate(const Vector2f& v);

	void SetCharacterVariable(const CharacterVariable& var) { m_charVars = var; }
	CharacterVariable GetCharacterVariable() const { return m_charVars; }

	void Update() override;

protected:
	void Move(const Vector3f& v, float delta);

protected:
	// Character movement variables
	Matrix3f			m_rotationStartMatrix;
	CharacterVariable	m_charVars;
	bool				m_rotationStarted;
	Vector2f			m_rotationStartPos;
	float				m_startTargetToH;
	Vector3f			m_lastTarget;

	// Flags for normal move & rotation
	uint32_t			m_moveFlag = 0;
	uint32_t			m_rotateFlag = 0;

	// Variables for mouse position interpolation
	// Since mouse position acquired from system is unstable in terms of both position & interval
	// Character needs to process them and produce smooth value in between just in order to make mouse rotation smooth
	bool				m_isControlInRotation = false;
	bool				m_isOperationInRotation = false;

	Vector2f			m_lastSampleCursorPosition;
	Vector2f			m_currentTargetPosition;
	Vector2f			m_rotationStartPosition;

	double				m_sampleInterval = 60.0f;
	double				m_elapsedSinceLastSample = 0.0f;
};