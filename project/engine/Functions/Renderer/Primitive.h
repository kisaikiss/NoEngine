#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {

class Primitive {
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
	static void DrawLine(const Math::Vector3& a, const Math::Vector3& b, const Math::Color& color);

	/// <summary>
	/// ボックス描画
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	/// <param name="viewProj">カメラ行列</param>
	static void DrawCube(const Math::Vector3& center, const Math::Vector3& size,
		const Math::Color& color);

	/// <summary>
	/// 球の描画
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	/// <param name="viewProj">カメラ行列</param>
	/// <param name="slices">分割数(x)</param>
	/// <param name="stacks">分割数(y)</param>
	static void DrawSphere(const Math::Vector3& center, float radius, const Math::Color& color, uint32_t slices = 16, uint32_t stacks = 16);

	/// <summary>
	/// 三角形描画
	/// </summary>
	/// <param name="a">ポイントA</param>
	/// <param name="b">ポイントB</param>
	/// <param name="c">ポイントC</param>
	/// <param name="color">色</param>
	/// <param name="viewProj">カメラ行列</param>
	static void DrawTriangle(const Math::Vector3& a, const Math::Vector3& b, const Math::Vector3& c, const Math::Color& color);

	/// <summary>
	/// 描画
	/// </summary>
	static void Render(GraphicsContext& ctx, const Math::Matrix4x4& ViewProj);

private:
	static void AddLineInternal(const Math::Vector3& a, const Math::Vector3& b, const Math::Color& color);
};
}