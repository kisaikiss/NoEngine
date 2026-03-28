#pragma once
#include "engine/NoEngine.h"

namespace CommentBoutCollision {

	struct CollisionDebugConfigComponent;

	/// <summary>
	/// 衝突デバッグ表示の統合System。
	/// 敵/自機/フィールドの当たり判定可視化を1か所で実行する。
	/// </summary>
	class CollisionDebugRenderSystem : public No::ISystem {
	public:
		CollisionDebugRenderSystem() { SetStopInPause(false); }
		void Update(No::Registry& registry, float deltaTime) override;

	private:
		CollisionDebugConfigComponent* EnsureConfig(No::Registry& registry);
		void DrawConfigImGui(CollisionDebugConfigComponent& config);
		void DrawEnemyColliderDebug(No::Registry& registry, const CollisionDebugConfigComponent& config);		//敵の当たり判定表示
		void DrawFieldColliderDebug(No::Registry& registry, const CollisionDebugConfigComponent& config);		//フィールドの当たり判定表示
		void DrawPlayerHitboxDebug(No::Registry& registry, const CollisionDebugConfigComponent& config);		//自機の当たり判定表示
		void DrawPlayerAttackColliderDebug(No::Registry& registry, const CollisionDebugConfigComponent& config);	//攻撃の当たり判定表示
		void DrawCameraGateDebug(No::Registry& registry, const CollisionDebugConfigComponent& config);			//カメラゲート(自機のあたる3D範囲の表示)
	};
}
