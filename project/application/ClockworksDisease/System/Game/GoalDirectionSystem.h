#pragma once
#include "engine/NoEngine.h"

// ゴール演出の進行(演出用カメラ/ゴールオブジェクトのTransformRoutineComponent再生完了待ち)を管理し、
// 完了したらGameClearSceneへフェード遷移するSystem。
// 実際の演出開始はItemGetSystem側が行う。
class GoalDirectionSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};