#pragma once

class IFrameEventListener
{
public:
	virtual void OnFrameBegin() = 0;
	virtual void OnPostSceneTraversal() = 0;
	virtual void OnPreCmdPreparation() = 0;
	virtual void OnPreCmdSubmission() = 0;
	virtual void OnFrameEnd() = 0;
};