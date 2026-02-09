#pragma once
#include "../Matrix4x4.h"
#include "../Vector3.h"
#include "../Quaternion.h"
namespace NoEngine {
namespace MathCalculations {
/// <summary>
/// 単位行列を作る
/// </summary>
/// <returns>単位行列</returns>
Math::Matrix4x4 MakeIdentity4x4();

/// <summary>
/// 行列の積
/// </summary>
/// <param name="matrix1"></param>
/// <param name="matrix2"></param>
/// <returns></returns>
Math::Matrix4x4 Multiply(Math::Matrix4x4 const& matrix1, Math::Matrix4x4 const& matrix2);

/// <summary>
/// 拡縮行列を作る
/// </summary>
/// <param name="scale"></param>
/// <returns></returns>
Math::Matrix4x4 MakeScaleMatrix(const Math::Vector3& scale);

/// <summary>
/// X軸方向の回転行列を作る
/// </summary>
/// <param name="theta"></param>
/// <returns></returns>
Math::Matrix4x4 MakePitchMatrix(float theta);

/// <summary>
/// Y軸方向の回転行列を作る
/// </summary>
/// <param name="theta"></param>
/// <returns></returns>
Math::Matrix4x4 MakeYowMatrix(float theta);

/// <summary>
/// Z軸方向の回転行列を作る
/// </summary>
/// <param name="theta"></param>
/// <returns></returns>
Math::Matrix4x4 MakeRollMatrix(float theta);


/// <summary>
/// X,Y,Z全て回転する回転行列を作る
/// </summary>
/// <param name="theta"></param>
/// <returns></returns>
Math::Matrix4x4 MakeRotateMatrix(const Math::Vector3& theta);

/// <summary>
/// 移動行列を作る
/// </summary>
/// <param name="translate"></param>
/// <returns></returns>
Math::Matrix4x4 MakeTranslateMatrix(const Math::Vector3& translate);

/// <summary>
/// アフィン変換行列
/// </summary>
/// <param name="scale"></param>
/// <param name="rotate"></param>
/// <param name="translate"></param>
/// <returns></returns>
Math::Matrix4x4 MakeAffineMatrix(const Math::Vector3& scale, const Math::Vector3& rotate, const Math::Vector3& translate);

Math::Matrix4x4 MakeAffineMatrix(const Math::Vector3& scale, const Math::Quaternion& rotate, const Math::Vector3& translate);

/// <summary>
/// 同次座標の行列を3次元座標に直す
/// </summary>
/// <param name="vector"></param>
/// <param name="matrix"></param>
/// <returns></returns>
Math::Vector3 Transform(const Math::Vector3& vector, const Math::Matrix4x4& matrix);

/// <summary>
/// 
/// </summary>
/// <param name="vector"></param>
/// <param name="matrix"></param>
/// <returns></returns>
Math::Vector3 TransforNormal(const Math::Vector3& vector, const Math::Matrix4x4& matrix);

/// <summary>
/// 逆行列
/// </summary>
/// <param name="matrix"></param>
/// <returns></returns>
Math::Matrix4x4 Inverse(Math::Matrix4x4 matrix);

/// <summary>
/// 転置行列
/// </summary>
/// <param name="matrix"></param>
/// <returns></returns>
Math::Matrix4x4 Transpose(Math::Matrix4x4 matrix);

/// <summary>
/// 正射影行列
/// </summary>
/// <param name="left"></param>
/// <param name="top"></param>
/// <param name="right"></param>
/// <param name="bottom"></param>
/// <returns></returns>
Math::Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float zNear, float zFar);

/// <summary>
/// 透視投影行列
/// </summary>
/// <param name="fovY"></param>
/// <param name="aspectRatio"></param>
/// <param name="nearClip"></param>
/// <param name="farClip"></param>
/// <returns></returns>
Math::Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

/// <summary>
/// ビューポート変換
/// </summary>
/// <param name="left"></param>
/// <param name="top"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="minDepth"></param>
/// <param name="maxDepth"></param>
/// <returns></returns>
Math::Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
}
}