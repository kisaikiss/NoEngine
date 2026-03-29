#pragma once
#include "engine/NoEngine.h"

class FieldObjectEditor;

class FieldEditorSystem : public No::ISystem {
public:
	explicit FieldEditorSystem(FieldObjectEditor* editor)
		: editor_(editor) { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
private:
	FieldObjectEditor* editor_ = nullptr;
};
