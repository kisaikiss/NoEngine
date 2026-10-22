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
	No::GroundStateComponent* groundState, bool isGrounded, PlayerMoveTransientComponent* transientState) {

	if (!No::InputIsTrigger("Jump")) {
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

	if (registry.Has<MultiJumpTag>(entity) && playerVariables->stamina >= 1.0f) {
		constexpr float kMultiJumpCost = 1.0f;
		playerVariables->yVelocity = playerVariables->doubleJumpSpeed;
		playerVariables->jumpKind = PlayerJumpKind::kNormal;
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
		if (hasHighJumpTag && PlayerStates::Get(playerVariables->state).IsJumpState() && !isGrounded) {
			if (playerVariables->stamina > playerVariables->highJumpCostRate * deltaTime) {
				playerVariables->yVelocity = playerVariables->highJumpSpeed;
				playerVariables->jumpKind = PlayerJumpKind::kHighJump;
				playerVariables->stamina -= playerVariables->highJumpCostRate * deltaTime;
			}
		}
	}

	if (No::InputIsTrigger("HighJump") && hasHighJumpTag) {
		if (isGrounded || playerVariables->infinityJump) {
			if (playerVariables->stamina > playerVariables->highJumpCostRate * deltaTime) {
				playerVariables->yVelocity = playerVariables->highJumpSpeed;
				playerVariables->jumpKind = PlayerJumpKind::kHighJump;
				transientState->justJumped = true;
				groundState->isGrounded = false;
			}
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

	if (playerVariables->stamina <= playerVariables->airDashStaminaCostRate * deltaTime) {
		return;
	}

	transientState->isAirDashing = true;
	playerVariables->stamina -= playerVariables->airDashStaminaCostRate * deltaTime;
}

// ジャンプ/ハイジャンプ入力を離した瞬間の減速処理
void HandleJumpRelease(PlayerComponent* playerVariables) {
	if (No::InputIsRelease("Jump") || No::InputIsRelease("HighJump")) {
		if (playerVariables->yVelocity > 0.f) {
			playerVariables->yVelocity *= 0.5f;
		}
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
		playerVariables->jumpKind = PlayerJumpKind::kNormal;
		playerVariables->canCreateScaffold = false;
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

	auto view = registry.View<PlayerComponent, No::TransformComponent, No::GroundStateComponent, PlayerMoveTransientComponent>();
	for (auto entity : view) {
		if (isSkip) continue;
		// リスポーン位置への移動が完了するまでは操作を止める。
		auto* deathSequence = registry.GetComponent<PlayerDeathSequenceComponent>(entity);
		if (deathSequence && deathSequence->phase != PlayerDeathSequenceComponent::Phase::kBlackOut &&
			deathSequence->phase != PlayerDeathSequenceComponent::Phase::kFadeIn) continue;

		if (registry.Has<GoalDirectionLockTag>(entity) || registry.Has<DialogueLockTag>(entity)) continue;

		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);

		if (No::InputIsRelease("AirDash") && transientState->isAirDashing) {
			playerVariables->yVelocity = 0.0f;
		}

		// このフレームの一時状態をリセット（このシステムが一番最初に触るため）
		transientState->justJumped = false;
		transientState->slopeY = 0.f;
		transientState->isAirDashing = false;

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

		HandleNormalJump(registry, entity, playerVariables, groundState, isGrounded, transientState);
		HandleHighJump(registry, entity, playerVariables, groundState, isGrounded, transientState, deltaTime);
		HandleAirDash(registry, entity, playerVariables, isGrounded, transientState, deltaTime);
		HandleJumpRelease(playerVariables);
		CreateScaffold(registry, entity, playerVariables, groundState, registry.GetComponent<No::TransformComponent>(entity));

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
