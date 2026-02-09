#pragma once
#include <deque>
#include "engine/Math/MathInclude.h"
#include "engine/NoEngine.h"

struct BallTrailComponent
{
    struct Sample {
        No::Vector3 pos;
        No::Color color = No::Color::WHITE;
        float age; // seconds since sample
    };
    std::deque<Sample> samples;

    // Parameters
    float maxAge = 0.6f;        // トレイルが消えるまでの時間
    float sampleInterval = 0.02f; // サンプリング間隔
    float timeSinceLast = 0.0f;
    float thickness = 0.35f;    // 基本厚さ（ワールド単位）
    uint32_t maxSamples = 64;

    // 頂点カラー向け設定
    No::Color startColor = No::Color(0xE491C9ff);
    No::Color endColor = No::Color(0x088395ff);
    uint32_t colorSegments = 4;
    bool useSegmentedColors = true;
};