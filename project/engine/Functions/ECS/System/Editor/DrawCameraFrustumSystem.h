#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {

// ActiveCameraTagが付いたカメラの視錐台(Frustum)を
// Near面/Far面を結ぶ線分(台形)でデバッグ描画するSystem。
// TransformRoutineComponentを併せ持つ場合は、各keyframeの位置・回転でも視錐台を描画する。
class DrawCameraFrustumSystem :
	public ISystem {
public:
	DrawCameraFrustumSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
};

}
}