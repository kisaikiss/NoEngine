#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {

// TransformRoutineComponentを持つエンティティのwaypoint同士を線で結んで描画するSystem。
// DebugPrimitive::DrawLineはどこで呼んでも描画キューに積むだけなので、
// 見た目の確認用にUpdateの中で毎フレーム呼び出す。
class DrawWaypointRouteSystem :
	public ISystem {
public:
	DrawWaypointRouteSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
};

}
}