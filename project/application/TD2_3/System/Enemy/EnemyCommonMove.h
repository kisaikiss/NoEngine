#pragma once
#include"engine/NoEngine.h"

/// @brief PoyoPyoアニメーション
/// @param transform 位置
/// @param timer アニメーションタイマー
/// @param speed PoyoPoyo周期
/// @param defaultScale デフォルトの大きさ
void PoyoPoyo(No::TransformComponent& transform, float timer, float speed, float scaling, const No::Vector3& defaultScale = No::Vector3::UNIT_SCALE);

void TimerUpdate(float& timer, const float& deltaTime);

void LookTarget(No::TransformComponent& transform, const No::Vector3& target);

No::Vector3 GatTargetDir(No::Vector3& translate, const No::Vector3& target);