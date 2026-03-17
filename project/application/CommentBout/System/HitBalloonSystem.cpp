#include "stdafx.h"
#include "HitBalloonSystem.h"
#include "application/CommentBout/Component/HitBalloonComponent.h"
#include "application/TestApp/Component/ProjectedColliderComponent.h"

void HitBalloonSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	auto view = registry.View<HitBalloonComponent, No::Transform2DComponent>();
	for (auto entity : view) {
		auto* balloon = registry.GetComponent<HitBalloonComponent>(entity);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!balloon || !transform) {
			continue;
		}

		// 追従先エンティティの投影コンポーネントを取得
		auto* projected = registry.GetComponent<TestApp::ProjectedColliderComponent>(balloon->sourceEntity);
		if (!projected || !projected->isVisible) {
			// 追従先が画面外なら吹き出しも非表示位置へ退避
			// （LifetimeSystemが削除するまでの間だけなので画面外座標で十分）
			transform->translate = { -9999.f, -9999.f };
			continue;
		}

		// アンカー種別に応じてスクリーン上の基点座標を決定
		No::Vector2 anchor{};
		switch (balloon->anchorType) {

		case HitBalloonComponent::AnchorType::TopRight:
			// 右上: スクリーン上での最右端 x, 最上端 y
			anchor = { projected->screenMax.x, projected->screenMin.y };
			break;

		case HitBalloonComponent::AnchorType::Top:
			// 真上中央: 投影中心 x, 最上端 y
			anchor = { projected->screenPosition.x, projected->screenMin.y };
			break;

		case HitBalloonComponent::AnchorType::TopLeft:
			// 左上: スクリーン上での最左端 x, 最上端 y
			anchor = { projected->screenMin.x, projected->screenMin.y };
			break;
		}

		// アンカー + 微調整オフセットを Transform2D に反映
		transform->translate = anchor + balloon->localOffset;
	}
}