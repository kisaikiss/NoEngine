#pragma once
#include "Types/Vector3.h"
#include "Types/Quaternion.h"

namespace NoEngine {
namespace Easing {
/// <summary>
/// 線形補間Vector3版
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="t"></param>
/// <returns></returns>
Vector3 Lerp(Vector3 start, Vector3 end, float t);

/// <summary>
/// 線形補間Quaternion版
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="t"></param>
/// <returns></returns>
Quaternion Lerp(Quaternion start, Quaternion end, float t);
}
}