#pragma once
#include "engine/Math/MathInclude.h"
namespace NoEngine {
struct TransformKeyframe : public Transform {
    float      duration = 1.0f;           // このwaypointに到達するまでの秒数
    Easing::EasingType easing = Easing::EasingType::Linear; // 直前waypoint→このwaypointの速度カーブ
};

namespace Component {
struct TransformRoutineComponent {
    std::vector<TransformKeyframe> keyframes;
    InterpolationType interpolation = InterpolationType::Linear;
    bool loop = true;
    bool playing = true;
    float playbackSpeed = 1.0f;

    // ランタイム状態（エディタでは基本触らない想定）
    int   currentIndex = 0;
    float elapsed = 0.0f;
};
}

}