#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// レール・カメラのデバッグライン描画を担当するシステム。
/// SetStopInPause(false) によりポーズ中も動作する。
/// </summary>
class RailCameraEditorSystem : public No::ISystem {
public:
	RailCameraEditorSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
