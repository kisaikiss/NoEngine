#include "PlayerComponent.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"

void RegisterPlayerGameplay() {
	using namespace NoEngine::Gameplay;
	auto& catalog = GameplayCatalog::Get();
	catalog.RegisterAction({ PlayerGameplayId::Lock, "Lock Player", "Player",
		[](No::Registry& registry, const NoEngine::Event::GameplayEvent&, No::Entity target, const nlohmann::json&) {
			if (!registry.Has<PlayerComponent>(target)) {
				LogWarning("Lock Player: target has no PlayerComponent");
				return;
			}
			if (!registry.Has<PlayerControlLockTag>(target)) registry.AddComponent<PlayerControlLockTag>(target);
			if (auto* input = registry.GetComponent<PlayerInputComponent>(target)) *input = {};
			if (auto* velocity = registry.GetComponent<No::VelocityComponent>(target)) {
				velocity->linear.x = 0.f;
				velocity->linear.z = 0.f;
			}
		}, true, {{ "targetEntity", "Target Entity", ActionPropertyType::EntityReference, true }} });
	catalog.RegisterAction({ PlayerGameplayId::Unlock, "Unlock Player", "Player",
		[](No::Registry& registry, const NoEngine::Event::GameplayEvent&, No::Entity target, const nlohmann::json&) {
			if (!registry.Has<PlayerComponent>(target)) {
				LogWarning("Unlock Player: target has no PlayerComponent");
				return;
			}
			if (registry.Has<PlayerControlLockTag>(target)) registry.RemoveComponent<PlayerControlLockTag>(target);
		}, true, {{ "targetEntity", "Target Entity", ActionPropertyType::EntityReference, true }} });
}

REFLECT_STRUCT_BEGIN(PlayerControlLockTag, "ApplicationTag")
REFLECT_STRUCT_END(PlayerControlLockTag)


REFLECT_STRUCT_BEGIN(PlayerComponent, "Application")
REFLECT_FIELD(moveSpeed),
REFLECT_FIELD(walkSpeed),
REFLECT_FIELD(dashStartInput),
REFLECT_FIELD(jumpSpeed),
REFLECT_FIELD(highJumpSpeed),
REFLECT_FIELD(doubleJumpSpeed),
REFLECT_FIELD(airDashSpeed),
REFLECT_FIELD(airDashDuration),
REFLECT_FIELD(spinAttackRadius),
REFLECT_FIELD(spinAttackAnimationSpeed),
REFLECT_FIELD(airAcceleration),
REFLECT_FIELD(accelerationTime),
REFLECT_FIELD(gravity),
REFLECT_FIELD(maxFallSpeed),
REFLECT_FIELD(stamina),
REFLECT_FIELD(maxStamina),
REFLECT_FIELD(staminaRecoveryRate),
REFLECT_FIELD(minGroundNormalY),
REFLECT_FIELD(coyoteTime),
REFLECT_FIELD(coyoteTimer),
REFLECT_FIELD(jumpBufferTime),
REFLECT_FIELD(infinityJump),
REFLECT_FIELD(infinityStamina),
REFLECT_ENUM_FIELD(state),
REFLECT_ENUM_FIELD(jumpKind),
REFLECT_FIELD(backflipStaminaCost),
REFLECT_FIELD(airDashStaminaCost),
REFLECT_FIELD(deathHeight),
REFLECT_FIELD(respawnPoint),
REFLECT_STRUCT_END(PlayerComponent)

REFLECT_STRUCT_BEGIN(PlayerAbilityDebugComponent, "Application")
REFLECT_FIELD(highJump),
REFLECT_FIELD(airDash),
REFLECT_FIELD(magicScaffold)
REFLECT_STRUCT_END(PlayerAbilityDebugComponent)

REFLECT_STRUCT_BEGIN(PlayerInputComponent, "ApplicationTag")
REFLECT_STRUCT_END(PlayerInputComponent)

REFLECT_STRUCT_BEGIN(PlayerActionComponent, "Application")
REFLECT_ENUM_FIELD(activeAction),
REFLECT_FIELD(elapsedTime)
REFLECT_STRUCT_END(PlayerActionComponent)

REFLECT_STRUCT_BEGIN(PlayerMoveTransientComponent, "ApplicationTag")
REFLECT_STRUCT_END(PlayerMoveTransientComponent)

REFLECT_STRUCT_BEGIN(LevelComponent, "Application")
REFLECT_FIELD(nowLevel),
REFLECT_FIELD(nextLevelUp),
REFLECT_FIELD(power),
REFLECT_ARRAY_FIELD(levelUpRequirements)
REFLECT_STRUCT_END(LevelComponent)
