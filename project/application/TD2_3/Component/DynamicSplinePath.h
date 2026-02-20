#pragma once
#include "engine/NoEngine.h"

struct PathComponent {
    std::deque<No::Vector3> controlPoints;  // 常に N 個保持
    float t = 0.0f;                     // 現在のスプライン上の位置 (0〜1)
    float speed = 1.0f;                 // 移動速度
};

No::Vector3 CatmullRom(const No::Vector3& p0, const No::Vector3& p1, const No::Vector3& p2, const No::Vector3& p3, float t);

/// <summary>
/// 画面上のランダムな位置(-5 ~ 5)を取得。zは0.0fです。
/// </summary>
/// <returns>ランダムな位置</returns>
No::Vector3 GetRandomPosition(No::Vector3 prePos, float minLength);
