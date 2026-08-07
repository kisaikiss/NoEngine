#pragma once
#include "EasingType.h"

namespace NoEngine {
namespace Easing {

template<typename T>
inline T Lerp(const T& start, const T& end, float t) {
	return start + (end - start) * t;
}

float ApplyEasing(EasingType type, float t);
}
}

