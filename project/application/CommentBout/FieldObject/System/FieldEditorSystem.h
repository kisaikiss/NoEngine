#pragma once
#include "engine/NoEngine.h"

class FieldEditorSystem : public No::ISystem {
public:
	FieldEditorSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
