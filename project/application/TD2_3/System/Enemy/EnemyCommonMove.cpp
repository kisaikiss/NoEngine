#include "EnemyCommonMove.h"
#include"engine/Math/MathInclude.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
using namespace No;
using namespace NoEngine;

void PoyoPoyo(No::TransformComponent& transform, float timer, float speed, float scaling, const NoEngine::Vector3& defaultScale)
{
    float theta = std::numbers::pi_v<float>*speed * timer;
    transform.scale.x = defaultScale.x + cos(theta) * scaling;
    transform.scale.y = defaultScale.y + sin(theta) * scaling;
}

void TimerUpdate(float& timer, const float& deltaTime)
{
    timer += deltaTime;
}

void LookTarget(No::TransformComponent& transform, const Vector3& target)
{
    Vector3 from = transform.translate;
    Vector3 to = target;

    using namespace MathCalculations;

    Vector3 forward = Normalize({ from.x - to.x, to.y - from.y, to.z - from.z });

    Vector3 right = Normalize(Cross(Vector3::UP, forward));
    Vector3 newUp = Cross(forward, right);

    // 回転行列 → クォータニオン変換
    float m00 = right.x, m01 = right.y, m02 = right.z;
    float m10 = newUp.x, m11 = newUp.y, m12 = newUp.z;
    float m20 = forward.x, m21 = forward.y, m22 = forward.z;

    float t = m00 + m11 + m22;

    Quaternion q;

    if (t > 0.0f) {
        float s = std::sqrt(t + 1.0f) * 2.0f;
        q.w = 0.25f * s;
        q.x = (m21 - m12) / s;
        q.y = (m02 - m20) / s;
        q.z = (m10 - m01) / s;
    } else if (m00 > m11 && m00 > m22) {
        float s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
        q.w = (m21 - m12) / s;
        q.x = 0.25f * s;
        q.y = (m01 + m10) / s;
        q.z = (m02 + m20) / s;
    } else if (m11 > m22) {
        float s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
        q.w = (m02 - m20) / s;
        q.x = (m01 + m10) / s;
        q.y = 0.25f * s;
        q.z = (m12 + m21) / s;
    } else {
        float s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
        q.w = (m10 - m01) / s;
        q.x = (m02 + m20) / s;
        q.y = (m12 + m21) / s;
        q.z = 0.25f * s;
    }

    transform.rotation = q;
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