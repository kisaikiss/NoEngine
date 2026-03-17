#include "stdafx.h"
#include "GrassReactionSystem.h"
#include "application/CommentBout/Component/GrassReactionComponent.h"
#include "application/CommentBout/Component/HitBalloonComponent.h"
#include "application/CommentBout/Component/LifetimeComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"

void GrassReactionSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	// 共有リソース（テクスチャなど）を取得
	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) {
			break;
		}
	}

	auto grassView = registry.View<CBGrassTag, GrassReactionComponent, TestApp::Collider3DComponent, TestApp::ProjectedColliderComponent>();
	for (auto entity : grassView) {
		auto* reaction = registry.GetComponent<GrassReactionComponent>(entity);
		auto* projected = registry.GetComponent<TestApp::ProjectedColliderComponent>(entity);
		if (!reaction || !projected) {
			continue;
		}

		// ---- 衝突開始を検出 -----------------------------------------------
		// projected->isColliding を参照する
		//   CheckProjectedVs2D が collider3D->isColliding と projected->isColliding
		//   を両方セットするため動いてはいたが、意味的に正しいのは projected 側
		// -------------------------------------------------------------------
		const bool hitNow = projected->isColliding;
		const bool hitPrev = reaction->wasColliding;

		if (hitNow && !hitPrev && projected->isVisible && gameResource) {

			// ---- アンカー計算 -----------------------------------------------
			// projected->screenMax.x = 画面上での最右端
			// projected->screenMin.y = 画面上での最上端（Yは下向き正なので値が小さい方が上）
			// これにより斜め視点でも常に「スクリーン上の右上」が基点になる
			// ---------------------------------------------------------------
		
			No::Vector2 anchor{ projected->screenMax.x, projected->screenMin.y };

			// エフェクトエンティティ生成
			auto effectEntity = registry.GenerateEntity();

			auto* transform2D = registry.AddComponent<No::Transform2DComponent>(effectEntity);
			// 初期位置はアンカーのみ（HitBalloonSystemが毎フレーム anchor+localOffset で上書きする）
			transform2D->translate = anchor;
			transform2D->scale = reaction->effectSize;

			auto* sprite = registry.AddComponent<No::SpriteComponent>(effectEntity);
			sprite->layer = reaction->effectLayer;
			sprite->textureHandle = gameResource->whiteTexture;
			sprite->color = { 0.0f, 1.0f, 0.0f, 1.0f };

			auto* lifetime = registry.AddComponent<LifetimeComponent>(effectEntity);
			lifetime->remainingTime = reaction->effectLifetime;

			// HitBalloonComponent: 3Dエンティティの投影位置に毎フレーム追従させる
			auto* balloon = registry.AddComponent<HitBalloonComponent>(effectEntity);
			balloon->sourceEntity = entity;
			// localOffset は HitBalloonComponent のデフォルト値 {0,0} を使用
			// 微調整したい場合は balloon->localOffset = { x, y }; で設定する
			balloon->anchorType = HitBalloonComponent::AnchorType::TopRight;

			registry.AddComponent<CBGrassHitEffectTag>(effectEntity);
		}

		// 次フレームの立ち上がり検出のため状態を保存
		// projected->isColliding で統一（collider3D->isColliding から変更）
		reaction->wasColliding = hitNow;
	}
}