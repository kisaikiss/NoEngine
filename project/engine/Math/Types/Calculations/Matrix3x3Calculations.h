#pragma once
#include "../Matrix3x3.h"
#include "../Vector2.h"

namespace NoEngine {
namespace MathCalculations {
/// <summary>
/// 逆行列にする関数
/// </summary>
/// <param name="matrix">逆行列にしたい行列</param>
/// <returns>逆行列</returns>
Matrix3x3 Inverse(Matrix3x3 const& matrix);

/// <summary>
/// 行列の乗算
/// </summary>
/// <param name="matrix1">行列1</param>
/// <param name="matrix2">行列2</param>
/// <returns>乗算結果</returns>
Matrix3x3 Multiply(Matrix3x3 const& matrix1, Matrix3x3 const& matrix2);

/// <summary>
/// 正射影行列を作る関数
/// </summary>
/// <param name="left">画面の左側</param>
/// <param name="top">画面の上側</param>
/// <param name="right">画面の右側</param>
/// <param name="bottom">画面の下側</param>
/// <returns>正射影行列</returns>
Matrix3x3 MakeOrthographicMatrix(float const& left, float const& top, float const& right, float const& bottom);

/// <summary>
/// ビューポート行列を作る
/// </summary>
/// <param name="left">左側</param>
/// <param name="top">上側</param>
/// <param name="width">画面の幅</param>
/// <param name="height">画面の高さ</param>
/// <returns></returns>
Matrix3x3 MakeViewportMatrix(float const& left, float const& top, float const& width, float const& height);

/// <summary>
/// 拡大縮小行列を作る
/// </summary>
/// <param name="scale">大きさの倍率</param>
/// <returns>拡大縮小行列</returns>
Matrix3x3 MakeScaleMatrix(Vector2 const& scale);

/// <summary>
/// 回転行列を作る
/// </summary>
/// <param name="theta">回転角</param>
/// <returns>回転行列</returns>
Matrix3x3 MakeRotateMatrix(float const& theta);

/// <summary>
/// 移動行列を作る
/// </summary>
/// <param name="translate">移動距離</param>
/// <returns>移動行列</returns>
Matrix3x3 MakeTranslateMatrix(Vector2 const& translate);

/// <summary>
/// アフィン変換する
/// </summary>
/// <param name="scale">大きさの倍率</param>
/// <param name="rotate">回転角</param>
/// <param name="translate">移動距離</param>
/// <returns>アフィン変換結果</returns>
Matrix3x3 MakeAffineMatrix(Vector2 const& scale, float const& rotate, Vector2 const& translate);

/// <summary>
/// 同次座標からデカルト座標に変換する
/// </summary>
/// <param name="vector">デカルト座標</param>
/// <param name="matrix">同次座標</param>
/// <returns>デカルト座標</returns>
Vector2 TransformPoint(Vector2 const& vector, Matrix3x3 const& matrix);
}
}