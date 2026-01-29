#pragma once
#include"engine/NoEngine.h"

/// @brief PoyoPyoアニメーション
/// @param transform 位置
/// @param timer アニメーションタイマー
/// @param speed PoyoPoyo周期
/// @param defaultScale デフォルトの大きさ
void PoyoPoyo(No::TransformComponent& transform, float timer, float speed, float scaling, const NoEngine::Vector3& defaultScale = NoEngine::Vector3::UNIT_SCALE);

void TimerUpdate(float& timer, const float& deltaTime);

void LookTarget(No::TransformComponent& transform, const NoEngine::Vector3& target);

NoEngine::Vector3 GatTargetDir(NoEngine::Vector3& translate, const NoEngine::Vector3& target);