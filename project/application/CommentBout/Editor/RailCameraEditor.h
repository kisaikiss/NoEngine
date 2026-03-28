#pragma once
#include "engine/NoEngine.h"

struct RailCameraComponent;

class RailCameraEditor {
public:
	using ResetEventRuntimeFn = void(*)(RailCameraComponent&);

	/// フローティングウィンドウ版 (ImGui::Begin/End 付き)
	void DrawRailCameraImGui(No::Registry* registry, No::Entity railCameraEntity, ResetEventRuntimeFn resetEventRuntime);
	void DrawRailEditorImGui(No::Registry* registry, No::Entity railCameraEntity);

	/// タブ内インライン版 (ImGui::Begin/End なし。タブアイテム内から呼ぶ)
	void DrawRailCameraContent(No::Registry* registry, No::Entity railCameraEntity, ResetEventRuntimeFn resetEventRuntime);
	void DrawRailEditorContent(No::Registry* registry, No::Entity railCameraEntity);

private:
	char stageNameBuffer_[64] = "";
};
