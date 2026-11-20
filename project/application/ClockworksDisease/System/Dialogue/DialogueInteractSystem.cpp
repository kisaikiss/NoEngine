#include "stdafx.h"
#include "DialogueInteractSystem.h"
#include "../../Component/Dialogue/DialogueComponent.h"
#include "../../Component/Player/PlayerComponent.h"

namespace {
// 実行時だけ生成する案内。話者が削除された場合も次の更新で破棄する。
struct DialogueInteractPrompt {
	No::Entity speaker = No::INVALID_ENTITY;
	float timer = 0.0f;
};

constexpr const char* kPromptTexture = "resources/game/ClockworksDisease/Texture/UI/XboxSeriesInput/Default/xbox_button_color_x.png";
constexpr const char* kPromptOutlineTexture = "resources/game/ClockworksDisease/Texture/UI/XboxSeriesInput/Default/xbox_button_color_x_outline.png";

No::Entity CreatePrompt(No::Registry& registry, No::Entity speaker) {
	const auto entity = registry.GenerateEntity();
	registry.AddComponent<DialogueInteractPrompt>(entity)->speaker = speaker;
	registry.AddComponent<No::TransformComponent>(entity);
	auto* billboard = registry.AddComponent<No::BillboardComponent>(entity);
	billboard->facing = NoEngine::Component::BillboardFacing::ScreenAligned;
	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	mesh->meshName = "resources/engine/Model/test/plane.obj";
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);
	material->textureName = kPromptTexture;
	material->blendMode = NoEngine::BlendMode::kAlpha;
	material->unlit = true;
	material->castShadow = false;
	material->receiveShadow = false;
	return entity;
}
}

void DialogueInteractSystem::Update(No::Registry& registry, float deltaTime) {
	const bool talkPressed = No::InputIsTrigger("Talk");
	std::vector<No::Entity> targets;

	for (auto player : registry.View<PlayerComponent, No::TransformComponent>()) {
		if (registry.Has<DialogueLockTag>(player) || registry.Has<PlayerControlLockTag>(player)) continue; // 会話中は新規に始めない
		No::Vector3 playerPos = registry.GetComponent<No::TransformComponent>(player)->GetWorldPosition(registry);

		// 範囲内で最も近い話し相手を選ぶ
		No::Entity nearest = No::INVALID_ENTITY;
		float best = 1e30f;
		for (auto e : registry.View<DialogueComponent, No::TransformComponent>()) {
			auto* dialogue = registry.GetComponent<DialogueComponent>(e);
			if (!dialogue->enabled || !dialogue->interactable || dialogue->lines.empty()) continue;

			float dist = (registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry) - playerPos).Length();
			if (dist <= dialogue->interactDistance && dist < best) {
				best = dist;
				nearest = e;
			}
		}
		if (nearest != No::INVALID_ENTITY) {
			if (talkPressed) {
				registry.EmitEvent(DialogueStartEvent{ nearest, player });
			} else if (std::find(targets.begin(), targets.end(), nearest) == targets.end()) {
				targets.push_back(nearest);
			}
		}
	}

	std::vector<No::Entity> prompts;
	for (auto entity : registry.View<DialogueInteractPrompt>()) {
		const auto speaker = registry.GetComponent<DialogueInteractPrompt>(entity)->speaker;
		const auto target = std::find(targets.begin(), targets.end(), speaker);
		if (target == targets.end()) {
			// 削除はフレーム末まで保留されるため、その場で描画も止める。
			if (auto* mesh = registry.GetComponent<No::MeshComponent>(entity)) mesh->isVisible = false;
			registry.DestroyEntity(entity);
		} else {
			prompts.push_back(entity);
			targets.erase(target);
		}
	}
	for (auto speaker : targets) prompts.push_back(CreatePrompt(registry, speaker));

	for (auto entity : prompts) {
		auto* prompt = registry.GetComponent<DialogueInteractPrompt>(entity);
		const auto* customSettings = registry.GetComponent<DialogueInteractPromptSettings>(prompt->speaker);
		const auto settings = customSettings ? *customSettings : DialogueInteractPromptSettings{};
		const auto position = registry.GetComponent<No::TransformComponent>(prompt->speaker)->GetWorldPosition(registry);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		transform->translate = position + settings.offset;
		// plane.objは一辺が2なので、指定した一辺のサイズに合わせる。
		const float scale = std::max(settings.size, 0.0f) * 0.5f;
		transform->scale = { scale, scale, 1.0f };
		const float interval = std::max(settings.interval, 0.01f);
		auto* material = registry.GetComponent<No::MaterialComponent>(entity);
		material->textureName = prompt->timer < interval ? kPromptTexture : kPromptOutlineTexture;
		prompt->timer = std::fmod(prompt->timer + std::max(deltaTime, 0.0f), interval * 2.0f);
	}
}

REGISTER_SYSTEM(::DialogueInteractSystem, "DialogueInteractSystem", "Gameplay")
