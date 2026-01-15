#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Math/MathInclude.h"

class Primitive
{
public:
	static void Initialize();
	static void Shutdown();

	/// <summary>
	/// ライン描画
	/// </summary>
	/// <param name="a">ポイントA</param>
	/// <param name="b">ポイントB</param>
	/// <param name="color">色</param>
	/// <param name="viewProj">カメラ行列</param>
	static void DrawLine(const NoEngine::Vector3& a, const NoEngine::Vector3& b, const NoEngine::Color& color);

	/// <summary>
	/// ボックス描画
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	/// <param name="viewProj">カメラ行列</param>
	static void DrawCube(const NoEngine::Vector3& center, const NoEngine::Vector3& size,
		const NoEngine::Color& color);

	/// <summary>
	/// 球の描画
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	/// <param name="viewProj">カメラ行列</param>
	/// <param name="slices">分割数(x)</param>
	/// <param name="stacks">分割数(y)</param>
	static void DrawSphere(const NoEngine::Vector3& center, float radius, const NoEngine::Color& color,uint32_t slices = 16, uint32_t stacks = 16);

	/// <summary>
	/// 三角形描画
	/// </summary>
	/// <param name="a">ポイントA</param>
	/// <param name="b">ポイントB</param>
	/// <param name="c">ポイントC</param>
	/// <param name="color">色</param>
	/// <param name="viewProj">カメラ行列</param>
	static void DrawTriangle(const NoEngine::Vector3& a, const NoEngine::Vector3& b, const NoEngine::Vector3& c, const NoEngine::Color& color);

	/// <summary>
	/// 描画
	/// </summary>
	static void Render(NoEngine::GraphicsContext& ctx);

	static void SetViewProj(const NoEngine::Matrix4x4& viewProj);
private:
	static void AddLineInternal(const NoEngine::Vector3& a, const NoEngine::Vector3& b, const NoEngine::Color& color);
};