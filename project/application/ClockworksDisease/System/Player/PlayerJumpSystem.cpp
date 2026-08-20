#include "PlayerJumpSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"

namespace {

// 通常ジャンプ・多段ジャンプのトリガー処理
void HandleNormalJump(No::Registry& registry, No::Entity entity, PlayerComponent* playerVariables,
	No::GroundStateComponent* groundState, bool isGrounded, PlayerMoveTransientComponent* transientState) {

	if (!No::InputIsTrigger("Jump")) {
		return;
	}

	if (isGrounded || playerVariables->infinityJump) {
		playerVariables->yVelocity = playerVariables->jumpSpeed;
		transientState->justJumped = true;
		// ジャンプ直後は接地フラグを即座に落とす
		// → このフレームの velocity.y 計算を「空中ブランチ」で処理させる
		groundState->isGrounded = false;
		return;
	}

	if (registry.Has<MultiJumpTag>(entity) && playerVariables->stamina >= 1.0f) {
		constexpr float kMultiJumpCost = 1.0f;
		playerVariables->yVelocity = playerVariables->doubleJumpSpeed;
		transientState->justJumped = true;
		groundState->isGrounded = false;
		playerVariables->stamina -= kMultiJumpCost;
	}
}

// ハイジャンプの「保持中の加速」と「発動トリガー」を担当
void HandleHighJump(No::Registry& registry, No::Entity entity, PlayerComponent* playerVariables,
	No::GroundStateComponent* groundState, bool isGrounded, PlayerMoveTransientComponent* transientState,
	float deltaTime) {

	const bool hasHighJumpTag = registry.Has<HighJumpTag>(entity);

	if (No::InputIsPress("HighJump")) {
		if (hasHighJumpTag && playerVariables->state == PlayerState::kHighJump && !isGrounded) {
			constexpr float kHighJumpDrainRate = 10.0f;
			if (playerVariables->stamina > kHighJumpDrainRate * deltaTime) {
				playerVariables->yVelocity = playerVariables->highJumpSpeed;
				playerVariables->stamina -= kHighJumpDrainRate * deltaTime;
			}
		}
	}

	if (No::InputIsTrigger("HighJump") && hasHighJumpTag) {
		if (isGrounded || playerVariables->infinityJump) {
			playerVariables->yVelocity = playerVariables->highJumpSpeed;
			transientState->justJumped = true;
			groundState->isGrounded = false;
		}
	}
}

// 空中で "AirDash" を押している間、スタミナを消費して空中ダッシュ状態にする。
// 実際の移動速度は PlayerHorizontalMoveSystem、重力無視は PlayerVerticalVelocitySystem が
// この isAirDashing フラグを見て処理する。
void HandleAirDash(No::Registry& registry, No::Entity entity, PlayerComponent* playerVariables,
	bool isGrounded, PlayerMoveTransientComponent* transientState, float deltaTime) {

	if (isGrounded || !registry.Has<AirDashTag>(entity) || !No::InputIsPress("AirDash")) {
		return;
	}

	constexpr float kAirDashStaminaCostRate = 2.0f; // 1秒あたりの消費量
	if (playerVariables->stamina <= kAirDashStaminaCostRate * deltaTime) {
		return;
	}

	transientState->isAirDashing = true;
	playerVariables->stamina -= kAirDashStaminaCostRate * deltaTime;
}

// ジャンプ/ハイジャンプ入力を離した瞬間の減速処理
void HandleJumpRelease(PlayerComponent* playerVariables) {
	if (No::InputIsRelease("Jump") || No::InputIsRelease("HighJump")) {
		if (playerVariables->yVelocity > 0.f) {
			playerVariables->yVelocity *= 0.5f;
		}
		playerVariables->state = PlayerState::kFall;
	}
}

// 足場を生成する処理
void CreateScaffold(No::Registry& registry, No::Entity entity, PlayerComponent* playerVariables,
	No::GroundStateComponent* groundState, No::TransformComponent* transform) {

	const bool hasCreateMagicScaffoldTag = registry.Has<CreateMagicScaffoldTag>(entity);
	if (hasCreateMagicScaffoldTag && playerVariables->canCreateScaffold) {
		for (auto e : registry.View<No::SpriteComponent, CanMagicUITag>()) {
			registry.GetComponent<No::SpriteComponent>(e)->isVisible = true;
		}
	} else {
		for (auto e : registry.View<No::SpriteComponent, CanMagicUITag>()) {
			if (!registry.GetComponent<CanMagicUITag>(e)->isBackground) {
				registry.GetComponent<No::SpriteComponent>(e)->isVisible = false;
			}
		}
	}

	if (No::InputIsTrigger("CreateScaffold")) {
		if (groundState->isGrounded || playerVariables->infinityJump || !playerVariables->canCreateScaffold || !hasCreateMagicScaffoldTag) {
			return;
		}


		// 足場をプレイヤーの下に生成する
		auto e = No::InstantiatePreset(registry, "resources/game/Prefabs/magicScaffold.json");
		auto* scaffoldTransform = registry.GetComponent<No::TransformComponent>(e);
		scaffoldTransform->translate = transform->translate;
		playerVariables->yVelocity = playerVariables->doubleJumpSpeed;
		playerVariables->canCreateScaffold = false;
	}
}

} // namespace

void PlayerJumpSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent, No::TransformComponent, No::GroundStateComponent, PlayerMoveTransientComponent>();
	for (auto entity : view) {
		if (registry.Has<GoalDirectionLockTag>(entity)) continue; // ゴール演出中は操作を受け付けない

		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);

		// このフレームの一時状態をリセット（このシステムが一番最初に触るため）
		transientState->justJumped = false;
		transientState->slopeY = 0.f;
		transientState->isAirDashing = false;

		// isGrounded は各ハンドラ内で isGrounded を書き換える前の値を使う必要があるため先に確定
		bool isGrounded = groundState->isGrounded;

		// コヨーテタイム内ならisGroundedをtrueにする
		if (!isGrounded && playerVariables->state != PlayerState::kJump) {
			if (playerVariables->coyoteTime > playerVariables->coyoteTimer) {
				playerVariables->coyoteTimer += deltaTime;
				isGrounded = true;
			}
		} else {
			if (playerVariables->state == PlayerState::kJump) {
				playerVariables->coyoteTimer = playerVariables->coyoteTime;
			} else {
				playerVariables->coyoteTimer = 0.0f;
			}
		}

		HandleNormalJump(registry, entity, playerVariables, groundState, isGrounded, transientState);
		HandleHighJump(registry, entity, playerVariables, groundState, isGrounded, transientState, deltaTime);
		HandleAirDash(registry, entity, playerVariables, isGrounded, transientState, deltaTime);
		HandleJumpRelease(playerVariables);
		CreateScaffold(registry, entity, playerVariables, groundState, registry.GetComponent<No::TransformComponent>(entity));
	}
}
