#include "InputHub.h"

void InputHub::ProcessKey(KeyState keyState, uint8_t keyCode)
{
	for (auto & pListener : m_registeredListener)
	{
		pListener->ProcessKey(keyState, keyCode);
	}
}

void InputHub::ProcessMouse(KeyState keyState, const Vector2f& mousePosition)
{
	for (auto & pListener : m_registeredListener)
	{
		pListener->ProcessMouse(keyState, mousePosition);
	}
}

void InputHub::ProcessMouse(const Vector2f& mousePosition)
{
	for (auto & pListener : m_registeredListener)
	{
		pListener->ProcessMouse(mousePosition);
	}
}

void InputHub::Register(const std::shared_ptr<IInputListener>& pListener)
{
	m_registeredListener.insert(pListener);
}