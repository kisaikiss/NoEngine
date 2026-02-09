#pragma once
#pragma once
#include "../Vector3.h"

namespace NoEngine {
namespace MathCalculations {
/// <summary>
/// ベクトルの内積
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
float Dot(Math::Vector3 v1, Math::Vector3 v2);

/// <summary>
/// ベクトルの外積
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
Math::Vector3 Cross(const Math::Vector3& v1, const Math::Vector3& v2);

/// <summary>
/// ベクトルの長さ
/// </summary>
/// <param name="vector3"></param>
/// <returns></returns>
float Length(Math::Vector3 vector3);

float LengthSquared(const Math::Vector3& v);

/// <summary>
/// ベクトルの正規化
/// </summary>
/// <param name="vector3"></param>
/// <returns></returns>
Math::Vector3 Normalize(const Math::Vector3& vector3);

/// <summary>
/// 正射影ベクトル(v1をv2に投影)
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
Math::Vector3 MakeOrthographicVector(const Math::Vector3& v1, const  Math::Vector3& v2);

/// <summary>
/// ベクトルの距離
/// </summary>
/// <param name="v1"></param>
/// <param name="v2"></param>
/// <returns></returns>
float Distance(const Math::Vector3& v1,const Math::Vector3& v2);
}
}