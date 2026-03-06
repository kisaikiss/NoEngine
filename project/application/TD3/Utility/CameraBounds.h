#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine {
	namespace ECS {
		class Registry;
	}
}

/// <summary>
/// カメラの可視範囲を計算するユーティリティ
/// </summary>
class CameraBounds {
public:
	/// <summary>
	/// カメラの可視範囲を取得する（指定されたZ位置での範囲）
	/// </summary>
	/// <param name="registry">レジストリ</param>
	/// <param name="zPosition">計算するZ位置（ワールド座標）</param>
	/// <param name="outLeft">左端のX座標（出力）</param>
	/// <param name="outRight">右端のX座標（出力）</param>
	/// <param name="outBottom">下端のY座標（出力）</param>
	/// <param name="outTop">上端のY座標（出力）</param>
	static void GetVisibleBounds(
		NoEngine::ECS::Registry& registry,
		float zPosition,
		float& outLeft,
		float& outRight,
		float& outBottom,
		float& outTop
	);

	/// <summary>
	/// 指定座標がカメラの可視範囲内にあるかチェック
	/// </summary>
	/// <param name="registry">レジストリ</param>
	/// <param name="worldPosition">チェックする座標</param>
	/// <param name="offset">判定範囲のオフセット（余裕を持たせる）</param>
	/// <returns>可視範囲内ならtrue</returns>
	static bool IsInBounds(
		NoEngine::ECS::Registry& registry,
		const NoEngine::Math::Vector3& worldPosition,
		float offset = 0.0f
	);

	/// <summary>
	/// 画面外に出た座標をループさせる（反対側に移動）
	/// </summary>
	/// <param name="registry">レジストリ</param>
	/// <param name="worldPosition">現在の座標</param>
	/// <param name="offset">判定範囲のオフセット</param>
	/// <param name="outLooped">ループしたかどうか（出力）</param>
	/// <returns>ループ後の座標</returns>
	static NoEngine::Math::Vector3 LoopPosition(
		NoEngine::ECS::Registry& registry,
		const NoEngine::Math::Vector3& worldPosition,
		float offset,
		bool& outLooped
	);
};
