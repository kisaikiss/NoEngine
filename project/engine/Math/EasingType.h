#pragma once

namespace NoEngine {

// waypoint間をどう繋ぐか（経路の形）
enum class InterpolationType {
    Linear,      // 直線
    CatmullRom,  // 前後のwaypointを考慮したスプライン
    Bezier,      // 制御点付きベジエ
    Step,        // 補間せず瞬間移動（アニメの「離散」演出用）
};

namespace Easing {

// 時間の進み方（速度カーブ）
enum class EasingType {
    Linear,
    EaseInQuad, EaseOutQuad, EaseInOutQuad,
    EaseInCubic, EaseOutCubic, EaseInOutCubic,
    EaseInBack, EaseOutBack, EaseInOutBack,
    EaseInSine, EaseOutSine, EaseInOutSine,
    EaseInQuart, EaseOutQuart, EaseInOutQuart,
    EaseInQuint, EaseOutQuint, EaseInOutQuint,
    EaseInExpo, EaesOutExpo, EaseInOutExpo,
    EaseInCirc, EaseOutCirc, EaseInOutCirc,
    EaseInElastic, EaseOutElastic, EaseInOutElastic,
};

}

} // namespace NoEngine