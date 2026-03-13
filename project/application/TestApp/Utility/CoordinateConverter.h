#pragma once
#include "engine/NoEngine.h"
#include "engine/Window/WindowSize.h"

namespace TestApp {

	/// <summary>
	/// 座標変換ユーティリティクラス
	/// 3Dワールド座標と2Dスクリーン座標の相互変換を提供する
	/// カメラの透視投影を考慮した正確な変換を行う
	/// </summary>
	class CoordinateConverter {
	public:
		/// <summary>
		/// 3Dワールド座標を2Dスクリーン座標に変換
		/// カメラのビュー・プロジェクション行列を使用して透視投影を適用する
		/// </summary>
		/// <param name="worldPos">ワールド座標での位置</param>
		/// <param name="viewProjection">カメラのビュー・プロジェクション行列</param>
		/// <param name="windowSize">ウィンドウのサイズ情報</param>
		/// <returns>
		/// スクリーン座標（ピクセル単位）
		/// カメラの後ろや画面外の場合は(-1000, -1000)を返す
		/// </returns>
		static No::Vector2 WorldToScreen(
			const No::Vector3& worldPos,
			const No::Matrix4x4& viewProjection,
			const NoEngine::WindowSize& windowSize
		);

		/// <summary>
		/// 3Dワールド空間の半径を2Dスクリーン空間の半径に変換
		/// 透視投影による距離補正を適用する（遠くのものは小さく、近くのものは大きく）
		/// </summary>
		/// <param name="worldCenter">ワールド座標での球体の中心位置</param>
		/// <param name="worldRadius">ワールド空間での半径</param>
		/// <param name="viewProjection">カメラのビュー・プロジェクション行列</param>
		/// <param name="windowSize">ウィンドウのサイズ情報</param>
		/// <returns>
		/// スクリーン空間での半径（ピクセル単位）
		/// カメラの後ろや画面外の場合は0.0fを返す
		/// </returns>
		static float WorldRadiusToScreen(
			const No::Vector3& worldCenter,
			float worldRadius,
			const No::Matrix4x4& viewProjection,
			const NoEngine::WindowSize& windowSize
		);

		/// <summary>
		/// スクリーン座標が画面内にあるかどうかを判定
		/// </summary>
		/// <param name="screenPos">スクリーン座標</param>
		/// <param name="windowSize">ウィンドウのサイズ情報</param>
		/// <returns>画面内にある場合true</returns>
		static bool IsOnScreen(
			const No::Vector2& screenPos,
			const NoEngine::WindowSize& windowSize
		);

		/// <summary>
		/// WorldToScreen の結果が有効な投影かどうかを判定
		/// clipW &lt;= 0（カメラの後ろ）や near/far クリッピングで無効になった場合 false
		/// 8頂点投影など複数点の有効性チェックに使用する
		/// </summary>
		/// <param name="screenPos">WorldToScreen の戻り値</param>
		/// <returns>有効な投影であれば true</returns>
		static bool IsValidProjection(const No::Vector2& screenPos);

	private:

		// 画面外を示す特殊な座標値
		static constexpr float kOffScreenValue = -1000.0f;

	};

}