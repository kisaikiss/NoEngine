#pragma once
#include "engine/NoEngine.h"

// TransformRoutineComponentによるカメラ演出(パス移動)が終わったら、
// 画面を暗転→復帰させつつCameraIntroLockTagを外し、
// FollowCameraSystemによるプレイヤー追従へ制御を渡すシステム。
class CameraIntroSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	No::Entity CreateFadeOverlay(No::Registry& registry);
	void SetOverlayAlpha(No::Registry& registry, No::Entity overlay, float alpha);
};