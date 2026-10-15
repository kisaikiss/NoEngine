#pragma once
#include "engine/Math/MathInclude.h"
#include "Transform2DComponent.h"

namespace NoEngine {
struct TransformKeyframe2D : public Component::Transform2DComponent {
    float      duration = 1.0f;           // このwaypointに到達するまでの秒数
    Easing::EasingType easing = Easing::EasingType::Linear; // 直前waypoint→このwaypointの速度カーブ
};

namespace Component {
struct TransformRoutineComponent2D {
    std::vector<TransformKeyframe2D> keyframes;
    // trueなら全区間に共通設定を使い、falseなら各waypointのduration/easingを使う
    bool useGlobalTiming = false;
    float globalDuration = 1.0f;
    Easing::EasingType globalEasing = Easing::EasingType::Linear;
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
