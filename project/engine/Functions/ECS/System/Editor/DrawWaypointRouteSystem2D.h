#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {

// TransformRoutineComponent2Dを持つエンティティのwaypoint同士を線で結んで描画するSystem(2D版)。
// DebugPrimitive::DrawLine2Dはどこで呼んでも描画キューに積むだけなので、
// 見た目の確認用にUpdateの中で毎フレーム呼び出す。
class DrawWaypointRouteSystem2D :
	public ISystem {
public:
	DrawWaypointRouteSystem2D() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
};

}
}