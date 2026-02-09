#pragma once
#include "engine/NoEngine.h"

namespace RNG {
float GetRandomValNormalized();
float GetRandomVal(float min, float max);
No::Vector3 GetRandomVector3(float min, float max);
}