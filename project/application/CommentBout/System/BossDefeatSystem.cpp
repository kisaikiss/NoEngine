#include "stdafx.h"
#include "BossDefeatSystem.h"
#include "application/CommentBout/Component/BossDefeatSequenceComponent.h"
#include "application/CommentBout/Component/ClearOverStateComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

namespace {
void ActivateClearOver(No::Registry& registry) {
	auto clearOverView = registry.View<CBClearOverStateTag, ClearOverStateComponent>();
	for (auto e : clearOverView) {
		auto* state = registry.GetComponent<ClearOverStateComponent>(e);
		if (state && state->phase == ClearOverStateComponent::Phase::Inactive) {
			state->result = ClearOverStateComponent::Result::Clear;
			state->phase = ClearOverStateComponent::Phase::FadeIn;
			state->phaseTimer = 0.f;
			state->selectedIndex = 0;
			state->isConfirmAnimating = false;
			state->confirmAnimTime = 0.f;
			state->confirmIndex = -1;
			state->requestedAction = ClearOverStateComponent::Action::None;
		}
		break;
	}
}

const GameResourceComponent* FindGameResource(No::Registry& registry) {
	auto gameResourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto e : gameResourceView) {
		return registry.GetComponent<GameResourceComponent>(e);
	}
	return nullptr;
}
}

void BossDefeatSystem::Update(No::Registry& registry, float deltaTime)
{
	BossDefeatSequenceComponent* defeat = nullptr;
	auto defeatView = registry.View<CBBossDefeatTag, BossDefeatSequenceComponent>();
	for (auto e : defeatView) {
		defeat = registry.GetComponent<BossDefeatSequenceComponent>(e);
		break;
	}
	if (!defeat || defeat->phase == BossDefeatSequenceComponent::Phase::Inactive ||
		defeat->phase == BossDefeatSequenceComponent::Phase::Done) {
		return;
	}

	defeat->phaseTimer += deltaTime;

	switch (defeat->phase) {
	case BossDefeatSequenceComponent::Phase::PreExplosionWait:
	{
		// プレイヤーを長時間無敵に設定
		auto playerView = registry.View<CBPlayerTag, InvincibleComponent>();
		for (auto playerEntity : playerView) {
			auto* inv = registry.GetComponent<InvincibleComponent>(playerEntity);
			if (inv) {
				inv->time = 9999.f;
			}
		}
		if (defeat->phaseTimer >= defeat->preExplosionDuration) {
			defeat->phase = BossDefeatSequenceComponent::Phase::Explosion;
			defeat->phaseTimer = 0.f;
		}
		break;
	}
	case BossDefeatSequenceComponent::Phase::Explosion:
	{
		// BOOM スプライトを生成 (初回のみ)
		if (defeat->effectSpriteEntity == No::nullEntity) {
			const GameResourceComponent* gameResource = FindGameResource(registry);
			auto effectEntity = registry.GenerateEntity();
			auto* transform = registry.AddComponent<No::Transform2DComponent>(effectEntity);
			transform->translate = { 640.f, 360.f };
			transform->scale = { 280.f, 280.f };
			auto* sprite = registry.AddComponent<No::SpriteComponent>(effectEntity);
			sprite->layer = CommentBout::ToLayer(CommentBout::SpriteLayer::Effect);
			sprite->isVisible = true;
			sprite->color = { 1.f, 1.f, 1.f, 1.f };
			if (gameResource) {
				sprite->textureHandle = GetGameTextureOrWhite(*gameResource, CommentBoutResourceKey::kSpeechBubbleMedium);
			}
			defeat->effectSpriteEntity = effectEntity;
		}

		if (defeat->phaseTimer >= defeat->explosionDuration) {
			// ボスエンティティを削除
			if (defeat->bossEntity != No::nullEntity && registry.Has<CBBossTag>(defeat->bossEntity)) {
				registry.DestroyEntity(defeat->bossEntity);
				defeat->bossEntity = No::nullEntity;
			}
			// BOOM スプライトを削除
			if (defeat->effectSpriteEntity != No::nullEntity) {
				registry.DestroyEntity(defeat->effectSpriteEntity);
				defeat->effectSpriteEntity = No::nullEntity;
			}
			defeat->phase = BossDefeatSequenceComponent::Phase::PostExplosionWait;
			defeat->phaseTimer = 0.f;
		}
		break;
	}
	case BossDefeatSequenceComponent::Phase::PostExplosionWait:
	{
		if (defeat->phaseTimer >= defeat->postExplosionDuration) {
			defeat->phase = BossDefeatSequenceComponent::Phase::Done;
			ActivateClearOver(registry);
		}
		break;
	}
	default:
		break;
	}
}
