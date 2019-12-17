#pragma once

#include "../common/Singleton.h"
#include "../Maths/Vector.h"
#include <memory>
#include <set>

enum KeyState
{
	KEY_DOWN,
	KEY_UP,
	KEY_STATE_COUNT
};

class IInputListener
{
public:
	virtual void ProcessKey(KeyState keyState, uint8_t keyCode) = 0;
	virtual void ProcessMouse(KeyState keyState, const Vector2d& mousePosition) = 0;
	virtual void ProcessMouse(const Vector2d& mousePosition) = 0;
};

class InputHub : public Singleton<InputHub>
{
public:
	double SampleInterval = 60.0f;

public:
	void ProcessKey(KeyState keyState, uint8_t keyCode);
	void ProcessMouse(KeyState keyState, const Vector2d& mousePosition);
	void ProcessMouse(const Vector2d& mousePosition);

public:
	void Register(const std::shared_ptr<IInputListener>& pListener);

private:
	std::set<std::shared_ptr<IInputListener>>	m_registeredListener;
};