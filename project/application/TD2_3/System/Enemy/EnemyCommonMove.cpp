#include "EnemyCommonMove.h"
#include"engine/Math/MathInclude.h"
using namespace No;
using namespace NoEngine;

void PoyoPoyo(No::TransformComponent& transform, float timer, float speed, float scaling, const NoEngine::Vector3& defaultScale)
{
    float theta = std::numbers::pi_v<float>*speed * timer;
    transform.scale.x = defaultScale.x + cos(theta) * scaling;
    transform.scale.y = defaultScale.y + sin(theta) * scaling;
}

void TimerUpdate(float& timer, float& deltaTime)
{
    timer += deltaTime;
}

void LookTarget(No::TransformComponent& transform, const Vector3& target)
{
    Vector3 direction = target - transform.translate;
    direction.z = 0.0f;
    // 正規化して方向ベクトルにする
    direction = direction.Normalize();

    float angle = std::atan2(direction.y, direction.x);

    transform.rotation.FromAxisAngle(Vector3::UP, PI + angle);

}


Vector3 GatTargetDir(Vector3& translate, const Vector3& target)
{
    Vector3 direction = target - translate;

    direction.z = 0.0f;

    float length = direction.Length();

    if (length != 0) {
        direction /= length;
    } else {
        direction = { Vector3::ZERO };
    }

    return direction;
}