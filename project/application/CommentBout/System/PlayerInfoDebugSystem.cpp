#include "stdafx.h"
#include "PlayerInfoDebugSystem.h"
#include "application/CommentBout/Component/PlayerComponent.h"
#include "application/CommentBout/Component/PlayerAttackComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/GameTag.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

void PlayerInfoDebugSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

#ifdef USE_IMGUI
	ImGui::Begin("Player Info");
	ImGui::Text("[Player Runtime]");

	auto view = registry.View<CBPlayerTag, PlayerComponent, PlayerAttackComponent, HealthComponent, InvincibleComponent, No::Transform2DComponent>();
	bool hasPlayer = false;
	for (auto entity : view) {
		hasPlayer = true;
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* attack = registry.GetComponent<PlayerAttackComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* invincible = registry.GetComponent<InvincibleComponent>(entity);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!player || !attack || !health || !invincible || !transform) {
			continue;
		}

		ImGui::Text("Entity: %llu", static_cast<unsigned long long>(entity));
		ImGui::Text("State: %s", health->isDead ? "Dead" : "Alive");
		ImGui::Text("HP: %d / %d", health->hp, health->maxHp);
		ImGui::Text("LastDamage: %d", health->lastDamageTaken);
		ImGui::Text("Invincible: %.2f / %.2f", invincible->time, invincible->duration);
		ImGui::Separator();
		ImGui::Text("MoveSpeed: %.1f", player->moveSpeed);
		ImGui::Text("Pos: (%.1f, %.1f)", transform->translate.x, transform->translate.y);
		ImGui::Text("Scale: (%.1f, %.1f)", transform->scale.x, transform->scale.y);
		ImGui::Separator();
		ImGui::Text("AttackPower: %d", attack->attackPower);
		ImGui::Text("AttackSize: (%.1f, %.1f)", attack->attackSize.x, attack->attackSize.y);
		ImGui::Text("AttackOffset: (%.1f, %.1f)", attack->spawnOffset.x, attack->spawnOffset.y);
		ImGui::Text("VisibleTime: %.2f", attack->visibleTime);
		break;
	}

	if (!hasPlayer) {
		ImGui::Text("Player not found.");
	}
	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}
