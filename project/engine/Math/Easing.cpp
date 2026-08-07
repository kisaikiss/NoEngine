#include "Easing.h"

namespace NoEngine {
namespace Easing {
float ApplyEasing(EasingType type, float t) {
	static constexpr float c1 = 1.70158f;
	static constexpr float c2 = c1 * 1.525f;
	static constexpr float c3 = c1 + 1.0f;
	static constexpr float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;
	static constexpr float c5 = (2.0f * std::numbers::pi_v<float>) / 4.5f;
	static constexpr float n1 = 7.5625f;
	static constexpr float d1 = 2.75f;
	switch (type) {
	case EasingType::Linear:
		return t;
		break;
	case EasingType::EaseInQuad:
		return t * t;
		break;
	case EasingType::EaseOutQuad:
		return 1.0f - (1.0f - t) * (1.0f - t);
		break;
	case EasingType::EaseInOutQuad:
		return t < 0.5f ? 2.0f * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
		break;
	case EasingType::EaseInCubic:
		return t * t * t;
		break;
	case EasingType::EaseOutCubic:
		return 1 - powf(1 - t, 3);
		break;
	case EasingType::EaseInOutCubic:
		return t < 0.5f ? 4.0f * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
		break;
	case EasingType::EaseInBack:
		return c3 * t * t * t - c1 * t * t;
		break;
	case EasingType::EaseOutBack:
		return 1 + c3 * powf(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
		break;
	case EasingType::EaseInOutBack:
		return t < 0.5f
			? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
			: (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
		break;
	case EasingType::EaseInSine:
		return 1.0f - cosf((t * std::numbers::pi_v<float>) / 2.0f);
		break;
	case EasingType::EaseOutSine:
		return sinf((t * std::numbers::pi_v<float>) / 2.0f);
		break;
	case EasingType::EaseInOutSine:
		return -(cosf(std::numbers::pi_v<float> *t) - 1.0f) / 2.0f;
		break;
	case EasingType::EaseInQuart:
		return  t * t * t * t;
		break;
	case EasingType::EaseOutQuart:
		return 1.0f - powf(1.0f - t, 4.0f);
		break;
	case EasingType::EaseInOutQuart:
		return t < 0.5f ? 8.0f * t * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
		break;
	case EasingType::EaseInQuint:
		return t * t * t * t * t;
		break;
	case EasingType::EaseOutQuint:
		return 1.0f - powf(1.0f - t, 5.0f);
		break;
	case EasingType::EaseInOutQuint:
		return  t < 0.5f
			? 16.0f * t * t * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
		break;
	case EasingType::EaseInExpo:
		return  t == 0.0f ? 0.0f
			: powf(2.0f, 10.0f * t - 10.0f);
		break;
	case EasingType::EaesOutExpo:
		return  t == 1.0f ? 1.0f
			: 1.0f - powf(2.0f, -10.0f * t);
		break;
	case EasingType::EaseInOutExpo:
		return t == 0.0f ? 0.0f
			: t == 1.0f ? 1.0f
			: t < 0.5f ? powf(2.0f, 20.0f * t - 10.0f) / 2.0f
			: (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
		break;
	case EasingType::EaseInCirc:
		return 1.0f - sqrtf(1.0f - powf(t, 2.0f));
		break;
	case EasingType::EaseOutCirc:
		return  sqrtf(1.0f - powf(t - 1.0f, 2.0f));
		break;
	case EasingType::EaseInOutCirc:
		return  t < 0.5f
			? (1.0f - sqrtf(1.0f - powf(2.0f * t, 2.0f))) / 2.0f
			: (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
		break;
	case EasingType::EaseInElastic:
		return  t == 0.0f ? 0.0f
			: t == 1.0f ? 1.0f
			: -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c4);
		break;
	case EasingType::EaseOutElastic:
		return t == 0.0f ? 0.0f
			: t == 1.0f ? 1.0f
			: powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
		break;
	case EasingType::EaseInOutElastic:
		return t == 0.0f ? 0.0f
			: t == 1.0f ? 1.0f
			: t < 0.5f
			? -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f
			: (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
		break;
	default:
		return t;
		break;
	}
}
}
}
