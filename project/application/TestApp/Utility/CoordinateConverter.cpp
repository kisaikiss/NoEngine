#include "CoordinateConverter.h"
#include <cmath>

namespace TestApp {

	No::Vector2 CoordinateConverter::WorldToScreen(
		const No::Vector3& worldPos,
		const No::Matrix4x4& viewProjection,
		const NoEngine::WindowSize& windowSize
	) {
		// ========================================
		// ワールド座標をクリップ座標に変換
		// ========================================

		// ビュー・プロジェクション行列を適用して4D同次座標を計算
		// [x, y, z, w] = [worldPos] * [viewProjection]

		float clipX = worldPos.x * viewProjection.m[0][0] +
			worldPos.y * viewProjection.m[1][0] +
			worldPos.z * viewProjection.m[2][0] +
			viewProjection.m[3][0];

		float clipY = worldPos.x * viewProjection.m[0][1] +
			worldPos.y * viewProjection.m[1][1] +
			worldPos.z * viewProjection.m[2][1] +
			viewProjection.m[3][1];

		float clipZ = worldPos.x * viewProjection.m[0][2] +
			worldPos.y * viewProjection.m[1][2] +
			worldPos.z * viewProjection.m[2][2] +
			viewProjection.m[3][2];

		float clipW = worldPos.x * viewProjection.m[0][3] +
			worldPos.y * viewProjection.m[1][3] +
			worldPos.z * viewProjection.m[2][3] +
			viewProjection.m[3][3];

		// ========================================
		// カメラの後ろにあるかチェック
		// ========================================
		// clipW <= 0 の場合、オブジェクトはカメラの後ろにある
		if (clipW <= 0.0f) {
			return { kOffScreenValue, kOffScreenValue };
		}

		// ========================================
		// クリップ座標をNDC（正規化デバイス座標）に変換
		// ========================================
		// NDC範囲: x, y, z は全て [-1, 1]
		// 透視除算: clip座標を w で割る

		float ndcX = clipX / clipW;
		float ndcY = clipY / clipW;
		float ndcZ = clipZ / clipW;

		// ========================================
		// クリッピング範囲外チェック
		// ========================================
		// NDCが [-1, 1] の範囲外の場合は画面外
		// Z軸は [0, 1] の範囲（DirectXの深度範囲）

		if (ndcZ < 0.0f || ndcZ > 1.0f) {
			return { kOffScreenValue, kOffScreenValue };
		}

		// ========================================
		// NDCをスクリーン座標に変換
		// ========================================
		// X軸: [-1, 1] → [0, windowWidth]
		// Y軸: [-1, 1] → [0, windowHeight] （上下反転）

		float screenX = (ndcX + 1.0f) * 0.5f * static_cast<float>(windowSize.clientWidth);
		float screenY = (1.0f - ndcY) * 0.5f * static_cast<float>(windowSize.clientHeight);

		return { screenX, screenY };
	}


	float CoordinateConverter::WorldRadiusToScreen(
		const No::Vector3& worldCenter,
		float worldRadius,
		const No::Matrix4x4& viewProjection,
		const NoEngine::WindowSize& windowSize
	) {
		// ========================================
		// 球体の中心をスクリーン座標に変換
		// ========================================

		No::Vector2 centerScreen = WorldToScreen(worldCenter, viewProjection, windowSize);

		// カメラの後ろや画面外の場合は半径0を返す
		if (centerScreen.x == kOffScreenValue || centerScreen.y == kOffScreenValue) {
			return 0.0f;
		}

		// ========================================
		// 球体の右端の点のワールド座標を計算
		// ========================================
		// 中心から右（X軸正方向）に半径分ずらした点

		No::Vector3 rightEdge = worldCenter;
		rightEdge.x += worldRadius;

		// ========================================
		// 右端の点をスクリーン座標に変換
		// ========================================

		No::Vector2 rightEdgeScreen = WorldToScreen(rightEdge, viewProjection, windowSize);

		// 右端も画面外の場合は半径0を返す
		if (rightEdgeScreen.x == kOffScreenValue || rightEdgeScreen.y == kOffScreenValue) {
			return 0.0f;
		}

		// ========================================
		// スクリーン座標での中心と右端の距離を計算
		// ========================================
		// これが透視投影補正された実際のスクリーン半径となる

		float dx = rightEdgeScreen.x - centerScreen.x;
		float dy = rightEdgeScreen.y - centerScreen.y;
		float screenRadius = std::sqrt(dx * dx + dy * dy);

		return screenRadius;
	}

	bool CoordinateConverter::IsOnScreen(
		const No::Vector2& screenPos,
		const NoEngine::WindowSize& windowSize
	) {
		// 特殊な画面外値のチェック
		if (screenPos.x == kOffScreenValue || screenPos.y == kOffScreenValue) {
			return false;
		}

		// 画面範囲内かチェック
		// 余裕を持たせるために少し広めに判定
		constexpr float margin = 100.0f;

		return screenPos.x >= -margin &&
			screenPos.x <= static_cast<float>(windowSize.clientWidth) + margin &&
			screenPos.y >= -margin &&
			screenPos.y <= static_cast<float>(windowSize.clientHeight) + margin;
	}

}
