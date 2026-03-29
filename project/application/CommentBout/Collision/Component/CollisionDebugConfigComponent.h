#pragma once

namespace CommentBoutCollision {
	/// <summary>
	/// 衝突デバッグ表示の統合設定。
	/// 敵/自機/フィールドの表示トグルを1か所で管理する。
	/// </summary>
	struct CollisionDebugConfigComponent {
		bool enableCollisionDebug = false;
		bool showEnemyCollider = true;
		bool showEnemyBulletCollider = true;
		bool showFieldCollider = true;
		bool showPlayerHitboxOverlay = true;
		bool showCameraGateWire = true;
		bool showPlayerAttackCollider = true;
	};

	/// <summary>
	/// 衝突デバッグ設定Entity識別タグ。
	/// </summary>
	struct CollisionDebugConfigTag {};
}
