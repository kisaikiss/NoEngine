#include "PlayerScaffoldActionSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "application/ClockworksDisease/Component/Dialogue/DialogueComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerDeathComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"

void PlayerScaffoldActionSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	bool cameraLocked = false;
	for (auto entity : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraLockTag>(entity)) cameraLocked = true;
	}

	auto view = registry.View<PlayerComponent, PlayerInputComponent, No::TransformComponent,
		No::GroundStateComponent>();
	for (auto entity : view) {
		if (cameraLocked) continue;
		const auto* death = registry.GetComponent<PlayerDeathSequenceComponent>(entity);
		if (death && death->phase != PlayerDeathSequenceComponent::Phase::kBlackOut
			&& death->phase != PlayerDeathSequenceComponent::Phase::kFadeIn) continue;
		if (registry.Has<GoalDirectionLockTag>(entity) || registry.Has<DialogueLockTag>(entity)) continue;

		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* input = registry.GetComponent<PlayerInputComponent>(entity);
		auto* ground = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		const bool hasAbility = registry.Has<CreateMagicScaffoldTag>(entity);

		if (hasAbility && player->canCreateScaffold) {
			for (auto uiEntity : registry.View<No::SpriteComponent, CanMagicUITag>()) {
				registry.GetComponent<No::SpriteComponent>(uiEntity)->isVisible = true;
			}
		} else {
			for (auto uiEntity : registry.View<No::SpriteComponent, CanMagicUITag>()) {
				if (!registry.GetComponent<CanMagicUITag>(uiEntity)->isBackground) {
					registry.GetComponent<No::SpriteComponent>(uiEntity)->isVisible = false;
				}
			}
		}

		if (!input->createScaffoldTriggered || ground->isGrounded || player->infinityJump
			|| !player->canCreateScaffold || !hasAbility) continue;

		const auto scaffold = No::InstantiatePreset(registry,
			"resources/game/Prefabs/magicScaffold.json");
		auto* scaffoldTransform = registry.GetComponent<No::TransformComponent>(scaffold);
		if (!scaffoldTransform) continue;
		scaffoldTransform->translate = transform->translate;
		player->yVelocity = player->doubleJumpSpeed;
		player->jumpKind = PlayerJumpKind::kNormal;
		player->canCreateScaffold = false;
	}
}

REGISTER_SYSTEM(::PlayerScaffoldActionSystem, "PlayerScaffoldActionSystem", "Gameplay")
