#include "PlayerJumpSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "PlayerStateMachine.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "../../Component/Camera/CameraIntroComponent.h"
#include "../../Component/Dialogue/DialogueComponent.h"
#include "../../Component/Player/PlayerDeathComponent.h"
#include "engine/Math/Collision/MeshColliderLoader.h"
#include "engine/Math/Collision/CollisionFanc.h"

namespace {
struct JumpLandingShadowTag {};

No::Entity EnsureJumpLandingShadow(No::Registry& registry) {
	No::Entity result = No::INVALID_ENTITY;
	for (auto entity : registry.View<JumpLandingShadowTag>()) {
		result = entity;
	}
	if (result != No::INVALID_ENTITY) {
		return result;
	}

	auto entity = registry.GenerateEntity();
	registry.AddComponent<JumpLandingShadowTag>(entity);
	registry.AddComponent<No::TransformComponent>(entity);
	auto* mesh = registry.AddComponent<No::MeshComponent>(entity);
	mesh->meshName = "resources/game/ClockworksDisease/Model/JumpShadow/shadowPlane.obj";
	mesh->isVisible = false;
	auto* material = registry.AddComponent<No::MaterialComponent>(entity);
	constexpr float shadowAlpha = 0.7f;
	material->color = No::Color{ 0.0f, 0.0f, 0.0f, shadowAlpha };
	material->blendMode = NoEngine::BlendMode::kAlpha;
	material->unlit = true;
	material->receiveShadow = false;
	material->castShadow = false;
	return entity;
}

bool FindGroundBelow(No::Registry& registry, const No::Vector3& playerPosition,
	No::RayTriangleHit& nearestHit) {
	constexpr float rayLength = 200.0f;
	constexpr float boundsPadding = 0.002f;
	const No::Vector3 rayOrigin = playerPosition + No::Vector3::UP * 0.1f;
	const No::Vector3 rayDirection{ 0.0f, -1.0f, 0.0f };
	nearestHit.distance = rayLength;
	bool foundHit = false;

	No::AABBCollider rayBounds;
	rayBounds.min = { playerPosition.x - boundsPadding,
		playerPosition.y - rayLength, playerPosition.z - boundsPadding };
	rayBounds.max = { playerPosition.x + boundsPadding,
		playerPosition.y + 0.1f, playerPosition.z + boundsPadding };

	for (auto terrainEntity : registry.View<No::TerrainMesh>()) {
		auto* terrain = registry.GetComponent<No::TerrainMesh>(terrainEntity);
		if (!terrain || !terrain->bvhRoot || terrain->triangles.empty()) continue;

		std::vector<int> candidates;
		No::QueryBVH(terrain->bvhRoot.get(), rayBounds, terrain->triangles, candidates);
		for (int index : candidates) {
			const auto hit = No::IntersectRayTriangle(rayOrigin, rayDirection,
				rayLength, terrain->triangles[static_cast<size_t>(index)]);
			if (!hit.hit || (foundHit && hit.distance >= nearestHit.distance)) continue;
			nearestHit = hit;
			foundHit = true;
		}
	}

	for (auto boxEntity : registry.View<No::TransformComponent, No::AABBCollider>()) {
		auto* transform = registry.GetComponent<No::TransformComponent>(boxEntity);
		auto* collider = registry.GetComponent<No::AABBCollider>(boxEntity);
		const auto worldBox = NoEngine::GetWorldAABB(transform, collider, registry);
		const No::Vector3 center = (worldBox.min + worldBox.max) * 0.5f;
		const No::Vector3 halfExtents = (worldBox.max - worldBox.min) * 0.5f;
		const auto hit = No::IntersectRayBox(rayOrigin, rayDirection, rayLength,
			center, halfExtents);
		if (hit.hit && (!foundHit || hit.distance < nearestHit.distance)) {
			nearestHit = hit;
			foundHit = true;
		}
	}

	for (auto boxEntity : registry.View<No::TransformComponent, No::OBBCollider>()) {
		auto* transform = registry.GetComponent<No::TransformComponent>(boxEntity);
		auto* collider = registry.GetComponent<No::OBBCollider>(boxEntity);
		const auto worldBox = NoEngine::GetWorldOBB(transform, collider, registry);
		const auto hit = No::IntersectRayBox(rayOrigin, rayDirection, rayLength,
			worldBox.center, worldBox.halfExtents, worldBox.rotation);
		if (hit.hit && (!foundHit || hit.distance < nearestHit.distance)) {
			nearestHit = hit;
			foundHit = true;
		}
	}
	return foundHit;
}
}

namespace {

// 通常ジャンプ・多段ジャンプのトリガー処理
void HandleNormalJump(No::Registry& registry, No::Entity entity, PlayerComponent* playerVariables,
	No::GroundStateComponent* groundState, bool isGrounded, PlayerMoveTransientComponent* transientState,
	const PlayerInputComponent& input) {

	if (!input.jumpTriggered && transientState->jumpBufferTimer <= 0.f) {
		return;
	}

	if (isGrounded || playerVariables->infinityJump) {
		playerVariables->yVelocity = playerVariables->jumpSpeed;
		playerVariables->jumpKind = PlayerJumpKind::kNormal;
		transientState->justJumped = true;
		// ジャンプ直後は接地フラグを即座に落とす
		// → このフレームの velocity.y 計算を「空中ブランチ」で処理させる
		groundState->isGrounded = false;
		return;
	}

	// 空中ジャンプは押した瞬間だけ実行し、先行入力が後からスタミナを消費するのを防ぐ。
	if (input.jumpTriggered && registry.Has<MultiJumpTag>(entity) && playerVariables->stamina >= 1.0f) {
		constexpr float kMultiJumpCost = 1.0f;
		playerVariables->yVelocity = playerVariables->doubleJumpSpeed;
		playerVariables->jumpKind = PlayerJumpKind::kNormal;
		transientState->justJumped = true;
		groundState->isGrounded = false;
		playerVariables->stamina -= kMultiJumpCost;
	}
}

// Backflipの30fpsキーフレーム。つま先が床を離れる直前と、床に触れる時刻。
constexpr float kBackflipTakeoff = 21.0f / 30.0f;
constexpr float kBackflipContact = 40.0f / 30.0f;
constexpr float kBackflipBeforeContact = 39.0f / 30.0f;

void ResetCrouchAndBackflip(PlayerMoveTransientComponent& move, No::AnimatorComponent* animator) {
	if (move.backflipPhase != PlayerBackflipPhase::kNone && animator) {
		animator->animationSpeedMagnification = 1.0f;
	}
	move.backflipPhase = PlayerBackflipPhase::kNone;
	move.crouching = false;
}

void UpdateBackflip(PlayerComponent& player, No::GroundStateComponent& ground,
	PlayerMoveTransientComponent& move, No::AnimatorComponent& animator, float deltaTime) {
	switch (move.backflipPhase) {
	case PlayerBackflipPhase::kWindup:
		if (!ground.isGrounded || !animator.IsPlaying()) {
			ResetCrouchAndBackflip(move, &animator);
		} else if (animator.time >= kBackflipTakeoff) {
			const float launchSpeed = std::max(player.highJumpSpeed, player.jumpSpeed * 1.15f);
			// 同じ高さに戻るまでの飛行時間に、クリップの空中区間を合わせる。
			const float flightTime = 2.0f * launchSpeed / std::max(-player.gravity, 0.01f);
			animator.Seek(kBackflipTakeoff);
			animator.animationSpeedMagnification = (kBackflipContact - kBackflipTakeoff) / flightTime;
			player.yVelocity = launchSpeed;
			player.coyoteTimer = player.coyoteTime;
			player.jumpKind = PlayerJumpKind::kHighJump;
			player.stamina = std::max(0.0f, player.stamina - std::max(player.backflipStaminaCost, 0.0f));
			ground.isGrounded = false;
			move.justJumped = true;
			move.backflipPhase = PlayerBackflipPhase::kAirborne;
		}
		break;
	case PlayerBackflipPhase::kAirborne:
		if (ground.isGrounded) {
			animator.Seek(kBackflipContact);
			animator.animationSpeedMagnification = 1.0f;
			player.yVelocity = 0.0f;
			move.backflipPhase = PlayerBackflipPhase::kLanding;
		} else if (animator.animationSpeedMagnification > 0.0f
			&& animator.time + animator.animationSpeedMagnification * deltaTime >= kBackflipBeforeContact) {
			// 低い足場へ落ちる場合は、接地するまで着地直前のポーズを保つ。
			animator.Seek(kBackflipBeforeContact);
			animator.animationSpeedMagnification = 0.0f;
		}
		break;
	case PlayerBackflipPhase::kLanding:
		if (!ground.isGrounded || !animator.IsPlaying()) {
			ResetCrouchAndBackflip(move, &animator);
		}
		break;
	default:
		break;
	}
}

void UpdateCrouchAndBackflip(No::Registry& registry, No::Entity entity, PlayerComponent& player,
	No::GroundStateComponent& ground, PlayerMoveTransientComponent& move,
	const PlayerInputComponent& input, No::AnimatorComponent* animator, float deltaTime) {
	if (move.backflipPhase != PlayerBackflipPhase::kNone) {
		if (animator) {
			UpdateBackflip(player, ground, move, *animator, deltaTime);
		} else {
			ResetCrouchAndBackflip(move, animator);
		}
		if (move.backflipPhase != PlayerBackflipPhase::kNone) return;
	}

	const auto* action = registry.GetComponent<PlayerActionComponent>(entity);
	move.crouching = ground.isGrounded && input.crouchPressed
		&& (!action || action->activeAction == PlayerActionType::kNone);
	if (!move.crouching || !input.jumpTriggered || !animator || !registry.Has<HighJumpTag>(entity)
		|| player.stamina < std::max(player.backflipStaminaCost, 0.0f)) return;

	const float previousSpeed = animator->animationSpeedMagnification;
	animator->animationSpeedMagnification = 1.0f;
	if (animator->Play("Backflip", false) && animator->GetDuration() > kBackflipContact) {
		move.backflipPhase = PlayerBackflipPhase::kWindup;
		move.crouching = false;
	} else {
		animator->animationSpeedMagnification = previousSpeed;
	}
}

} // namespace

void PlayerJumpSystem::Update(No::Registry& registry, float deltaTime) {
	const No::Entity shadowEntity = EnsureJumpLandingShadow(registry);
	auto* shadowMesh = registry.GetComponent<No::MeshComponent>(shadowEntity);
	auto* shadowTransform = registry.GetComponent<No::TransformComponent>(shadowEntity);
	if (shadowMesh) shadowMesh->isVisible = false;

	bool isSkip = false;
	for (auto e : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
		if (registry.Has<CameraLockTag>(e)) isSkip = true; // 演出中は操作できない
	}

	auto view = registry.View<PlayerComponent, PlayerInputComponent, No::TransformComponent,
		No::GroundStateComponent, PlayerMoveTransientComponent>();
	for (auto entity : view) {
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);
		transientState->justJumped = false;
		transientState->slopeY = 0.f;
		auto clearJumpBuffer = [&]() {
			transientState->jumpBufferTimer = 0.f;
			transientState->bufferedJumpReleased = false;
		};
		auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);
		// リスポーン位置への移動が完了するまでは操作を止める。
		const auto* deathSequence = registry.GetComponent<PlayerDeathSequenceComponent>(entity);
		const bool deathLocked = deathSequence
			&& deathSequence->phase != PlayerDeathSequenceComponent::Phase::kBlackOut
			&& deathSequence->phase != PlayerDeathSequenceComponent::Phase::kFadeIn;
		if (isSkip || deathLocked || registry.Has<GoalDirectionLockTag>(entity) || registry.Has<DialogueLockTag>(entity)) {
			clearJumpBuffer();
			ResetCrouchAndBackflip(*transientState, animator);
			continue;
		}

		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* input = registry.GetComponent<PlayerInputComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		UpdateCrouchAndBackflip(registry, entity, *playerVariables, *groundState, *transientState,
			*input, animator, deltaTime);
		if (transientState->crouching || transientState->backflipPhase != PlayerBackflipPhase::kNone) {
			if (groundState->isGrounded) playerVariables->coyoteTimer = 0.0f;
			clearJumpBuffer();
		} else {
			transientState->jumpBufferTimer = std::max(0.f,
				transientState->jumpBufferTimer - std::max(deltaTime, 0.f));
			if (input->jumpTriggered) {
				transientState->jumpBufferTimer = std::max(playerVariables->jumpBufferTime, 0.f);
				transientState->bufferedJumpReleased = false;
			}
			if (input->jumpReleased && transientState->jumpBufferTimer > 0.f) {
				transientState->bufferedJumpReleased = true;
			}

			// isGrounded は各ハンドラ内で isGrounded を書き換える前の値を使う必要があるため先に確定
			bool isGrounded = groundState->isGrounded;

			// コヨーテタイム内ならisGroundedをtrueにする
			if (!isGrounded && !PlayerStates::Get(playerVariables->state).IsJumpState()) {
				if (playerVariables->coyoteTime > playerVariables->coyoteTimer) {
					playerVariables->coyoteTimer += deltaTime;
					isGrounded = true;
				}
			} else {
				if (PlayerStates::Get(playerVariables->state).IsJumpState()) {
					playerVariables->coyoteTimer = playerVariables->coyoteTime;
				} else {
					playerVariables->coyoteTimer = 0.0f;
				}
			}

			HandleNormalJump(registry, entity, playerVariables, groundState, isGrounded, transientState, *input);
			if (transientState->justJumped) {
				// 入力を離したフレームの減速を二重に適用しない。
				if (transientState->bufferedJumpReleased && !input->jumpReleased) {
					playerVariables->yVelocity *= 0.5f;
				}
				clearJumpBuffer();
			}
			if (input->jumpReleased && playerVariables->yVelocity > 0.f) {
				playerVariables->yVelocity *= 0.5f;
			}
		}
		const bool isJumping = PlayerStates::Get(playerVariables->state).IsJumpState()
			|| transientState->justJumped || !groundState->isGrounded;
		if (!isJumping || !shadowMesh || !shadowTransform) continue;

		const auto* playerTransform = registry.GetComponent<No::TransformComponent>(entity);
		if (!playerTransform) continue;
		const No::Vector3 playerPosition = playerTransform->GetWorldPosition(registry);
		No::RayTriangleHit centerHit;
		if (!FindGroundBelow(registry, playerPosition, centerHit)) continue;

		// 周囲の地面サンプルから局所的な平均法線と凹凸量を求める。
		// 影自体は1枚だけにし、凹凸が強い場所では小さくして地形へのめり込みを抑える。
		No::Vector3 normalSum = centerHit.normal;
		float heightVariation = 0.0f;
		int normalSamples = 1;
		constexpr float sampleRadius = 0.5f;
		const No::Vector3 sampleOffsets[] = {
			{ sampleRadius, 0.0f, 0.0f }, { -sampleRadius, 0.0f, 0.0f },
			{ 0.0f, 0.0f, sampleRadius }, { 0.0f, 0.0f, -sampleRadius },
			{ sampleRadius * 0.7071f, 0.0f, sampleRadius * 0.7071f },
			{ -sampleRadius * 0.7071f, 0.0f, sampleRadius * 0.7071f },
			{ sampleRadius * 0.7071f, 0.0f, -sampleRadius * 0.7071f },
			{ -sampleRadius * 0.7071f, 0.0f, -sampleRadius * 0.7071f },
		};
		for (const auto& offset : sampleOffsets) {
			No::RayTriangleHit sampleHit;
			if (!FindGroundBelow(registry, playerPosition + offset, sampleHit)) continue;
			const float heightDelta = std::abs(sampleHit.position.y - centerHit.position.y);
			if (heightDelta > 0.45f) continue; // 段差の向こう側を平均法線に混ぜない
			heightVariation = std::max(heightVariation, heightDelta);
			normalSum += sampleHit.normal;
			++normalSamples;
		}

		No::Vector3 surfaceNormal = (normalSum / static_cast<float>(normalSamples)).Normalize();
		if (surfaceNormal.LengthSquared() < 1e-6f) surfaceNormal = centerHit.normal;
		float shadowScale = std::max(0.3f, 0.6f - heightVariation * 1.6f);
		constexpr float minimumShadowScale = 0.2f;
		constexpr int edgeProbeCount = 24;
		constexpr float pi = 3.14159265358979323846f;
		bool fitsOnGround = false;
		for (int attempt = 0; attempt < 8; ++attempt) {
			bool ringIsSupported = true;
			const float probeRadius = shadowScale * 1.05f;
			for (int probe = 0; probe < edgeProbeCount; ++probe) {
				const float angle = 2.0f * pi * static_cast<float>(probe) / edgeProbeCount;
				const No::Vector3 probePosition = playerPosition + No::Vector3{
					std::cos(angle) * probeRadius, 0.0f, std::sin(angle) * probeRadius };
				No::RayTriangleHit edgeHit;
				if (!FindGroundBelow(registry, probePosition, edgeHit)
					|| std::abs(edgeHit.position.y - centerHit.position.y) > 0.45f) {
					ringIsSupported = false;
					break;
				}
			}
			if (ringIsSupported) {
				fitsOnGround = true;
				break;
			}
			if (shadowScale <= minimumShadowScale) break;
			shadowScale = std::max(minimumShadowScale, shadowScale * 0.8f);
		}
		if (!fitsOnGround) continue;
		shadowTransform->translate = centerHit.position + surfaceNormal * 0.018f;
		shadowTransform->rotation.LookRotation(surfaceNormal,
			std::abs(surfaceNormal.y) > 0.99f ? No::Vector3::FORWARD : No::Vector3::UP);
		shadowTransform->scale = { shadowScale, shadowScale, 1.0f };
		shadowMesh->isVisible = true;
	}
}

REGISTER_SYSTEM(::PlayerJumpSystem, "PlayerJumpSystem", "Gameplay")
