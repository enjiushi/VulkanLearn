#pragma once

class IFrameEventListener
{
public:
	virtual void OnFrameBegin() = 0;
	virtual void OnFrameEnd() = 0;
};