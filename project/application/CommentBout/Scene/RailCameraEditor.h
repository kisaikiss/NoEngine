#pragma once
#include "engine/NoEngine.h"

struct RailCameraComponent;

class RailCameraEditor {
public:
	using ResetEventRuntimeFn = void(*)(RailCameraComponent&);

	void DrawRailCameraImGui(No::Registry* registry, No::Entity railCameraEntity, ResetEventRuntimeFn resetEventRuntime);
	void DrawRailEditorImGui(No::Registry* registry, No::Entity railCameraEntity);

private:
	char stageNameBuffer_[64] = "";
};
