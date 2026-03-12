#pragma once
#include "engine/NoEngine.h"

namespace TestApp {

	/// <summary>
	/// 衝突判定アルゴリズム集
	/// 2D/3Dの各種形状同士の衝突判定を提供する
	/// </summary>
	class CollisionAlgorithms {
	public:
		// ========================================
		// 2D衝突判定
		// ========================================

		/// <summary>
		/// 円と円の衝突判定
		/// 中心間の距離が半径の和以下かどうかで判定
		/// </summary>
		/// <param name="center1">円1の中心座標</param>
		/// <param name="radius1">円1の半径</param>
		/// <param name="center2">円2の中心座標</param>
		/// <param name="radius2">円2の半径</param>
		/// <returns>衝突している場合true</returns>
		static bool CheckCircleCircle(
			const No::Vector2& center1, float radius1,
			const No::Vector2& center2, float radius2
		);

		/// <summary>
		/// 円とAABB（軸平行境界ボックス）の衝突判定
		/// 円の中心からAABBの最近接点を求め、その距離が半径以下かどうかで判定
		/// </summary>
		/// <param name="circleCenter">円の中心座標</param>
		/// <param name="radius">円の半径</param>
		/// <param name="rectCenter">矩形の中心座標</param>
		/// <param name="rectSize">矩形のフルサイズ（幅・高さ）</param>
		/// <returns>衝突している場合true</returns>
		/// <remarks>
		/// rectSizeは内部で自動的にハーフサイズに変換される
		/// 例: rectSize = (100, 100) の場合、中心から±50の範囲が矩形となる
		/// </remarks>
		static bool CheckCircleAABB(
			const No::Vector2& circleCenter, float radius,
			const No::Vector2& rectCenter, const No::Vector2& rectSize
		);

		/// <summary>
		/// AABBとAABBの衝突判定
		/// 各軸で範囲が重なっているかどうかで判定
		/// </summary>
		/// <param name="center1">矩形1の中心座標</param>
		/// <param name="size1">矩形1のフルサイズ（幅・高さ）</param>
		/// <param name="center2">矩形2の中心座標</param>
		/// <param name="size2">矩形2のフルサイズ（幅・高さ）</param>
		/// <returns>衝突している場合true</returns>
		static bool CheckAABBAABB(
			const No::Vector2& center1, const No::Vector2& size1,
			const No::Vector2& center2, const No::Vector2& size2
		);

		// ========================================
		// 3D衝突判定
		// ========================================

		/// <summary>
		/// 球体と球体の衝突判定
		/// 中心間の距離が半径の和以下かどうかで判定
		/// </summary>
		/// <param name="center1">球体1の中心座標</param>
		/// <param name="radius1">球体1の半径</param>
		/// <param name="center2">球体2の中心座標</param>
		/// <param name="radius2">球体2の半径</param>
		/// <returns>衝突している場合true</returns>
		static bool CheckSphereSphere(
			const No::Vector3& center1, float radius1,
			const No::Vector3& center2, float radius2
		);

		/// <summary>
		/// 球体と3D AABB（軸平行境界ボックス）の衝突判定
		/// 球体の中心からAABBの最近接点を求め、その距離が半径以下かどうかで判定
		/// </summary>
		/// <param name="sphereCenter">球体の中心座標</param>
		/// <param name="radius">球体の半径</param>
		/// <param name="boxCenter">ボックスの中心座標</param>
		/// <param name="boxSize">ボックスのフルサイズ（幅・高さ・奥行き）</param>
		/// <returns>衝突している場合true</returns>
		static bool CheckSphereAABB3D(
			const No::Vector3& sphereCenter, float radius,
			const No::Vector3& boxCenter, const No::Vector3& boxSize
		);

		/// <summary>
		/// 3D AABB同士の衝突判定
		/// 各軸で範囲が重なっているかどうかで判定
		/// </summary>
		/// <param name="center1">ボックス1の中心座標</param>
		/// <param name="size1">ボックス1のフルサイズ（幅・高さ・奥行き）</param>
		/// <param name="center2">ボックス2の中心座標</param>
		/// <param name="size2">ボックス2のフルサイズ（幅・高さ・奥行き）</param>
		/// <returns>衝突している場合true</returns>
		static bool CheckAABB3DAABB3D(
			const No::Vector3& center1, const No::Vector3& size1,
			const No::Vector3& center2, const No::Vector3& size2
		);
	};

}