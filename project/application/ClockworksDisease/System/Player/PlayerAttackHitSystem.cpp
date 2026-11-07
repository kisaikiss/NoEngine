#include "stdafx.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerDeathComponent.h"
#include "application/ClockworksDisease/Component/Dialogue/DialogueComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"
#include "application/ClockworksDisease/System/Game/CollisionLayer.h"
#include "application/ClockworksDisease/System/Game/CollisionEvents.h"
#include "engine/Math/Collision/CollisionFanc.h"

namespace {
constexpr float kLaunchHorizontalSpeed = 4.5f;
constexpr float kLaunchUpSpeed = 5.0f;
constexpr float kDefeatGravity = 18.0f;
constexpr float kMaxFallSpeed = -20.0f;
constexpr float kBounceRetention = 0.42f;
constexpr float kTangentialRetention = 0.55f;
constexpr float kRestDuration = 0.25f;
constexpr float kMaxPhysicsStep = 1.0f / 120.0f;
constexpr float kMaxFallDistance = 100.0f;

struct EnemyDefeatMotionComponent {
	No::CapsuleCollider collider;
	No::Vector3 velocity;
	float launchHeight;
	bool settled = false;
	float restTime = 0.0f;
};

bool IsSolidPlatform(No::Registry& registry, No::Entity entity) {
	const auto* layer = registry.GetComponent<CollisionLayerComponent>(entity);
	if ((layer->layer & CollisionLayerComponent::Terrain) == CollisionLayerComponent::None) return false;
	const auto* body = registry.GetComponent<No::CollisionBody>(entity);
	return !body || body->type != NoEngine::Component::BodyType::Through;
}

void ResolvePlatformContact(No::Registry& registry, const No::CapsuleCollider& collider,
	No::Vector3& position, No::Vector3& normal) {
	NoEngine::Transform probe;
	probe.translate = position;
	float deepest = 0.0f;
	auto consider = [&](const auto& contact) {
		if (contact.hit && contact.penetration > deepest && contact.normal.LengthSquared() > 0.5f) {
			deepest = contact.penetration;
			normal = contact.normal;
		}
	};
	for (auto entity : registry.View<CollisionLayerComponent, No::TransformComponent, No::AABBCollider>()) {
		if (!IsSolidPlatform(registry, entity)) continue;
		consider(NoEngine::Math::TestCapsuleAABB(&probe, &collider,
			registry.GetComponent<No::TransformComponent>(entity), registry.GetComponent<No::AABBCollider>(entity), registry));
	}
	for (auto entity : registry.View<CollisionLayerComponent, No::TransformComponent, No::OBBCollider>()) {
		if (!IsSolidPlatform(registry, entity)) continue;
		consider(NoEngine::Math::TestCapsuleOBB(&probe, &collider,
			registry.GetComponent<No::TransformComponent>(entity), registry.GetComponent<No::OBBCollider>(entity), registry));
	}
	position += normal * deepest;
}

void Bounce(EnemyDefeatMotionComponent& motion, const No::Vector3& normal) {
	const float impactSpeed = -motion.velocity.Dot(normal);
	if (impactSpeed <= 0.0f) return;

	const auto tangent = motion.velocity + normal * impactSpeed;
	motion.velocity = tangent * kTangentialRetention + normal * (impactSpeed * kBounceRetention);
	if (normal.y >= 0.75f && impactSpeed < 1.1f && motion.velocity.LengthSquared() < 1.0f) {
		motion.velocity = No::Vector3::ZERO;
		motion.settled = true;
	}
}

void UpdateDefeatMotion(No::Registry& registry, No::Entity enemy, float deltaTime) {
	auto& motion = *registry.GetComponent<EnemyDefeatMotionComponent>(enemy);
	if (motion.settled) {
		motion.restTime += deltaTime;
		if (motion.restTime >= kRestDuration) registry.DestroyEntity(enemy);
		return;
	}

	auto& transform = *registry.GetComponent<No::TransformComponent>(enemy);
	auto position = transform.GetWorldPosition(registry);
	auto inverseParentRotation = No::Quaternion::IDENTITY;
	if (auto* parent = registry.GetComponent<No::TransformComponent>(transform.parent)) {
		inverseParentRotation = parent->GetWorldRotation(registry);
		inverseParentRotation.Inverse();
	}

	// 足場のカプセル接触は短い間隔で調べ、地形メッシュには移動区間全体の判定を使う。
	float remaining = std::min(deltaTime, 0.1f);
	while (remaining > 0.0f && !motion.settled) {
		const float step = std::min(remaining, kMaxPhysicsStep);
		remaining -= step;
		motion.velocity.y = std::max(kMaxFallSpeed, motion.velocity.y - kDefeatGravity * step);
		const auto displacement = motion.velocity * step;
		const NoEngine::CapsuleWorld capsule{position + motion.collider.localP0,
			position + motion.collider.localP1, motion.collider.radius};
		const auto terrain = NoEngine::Math::SweepCapsuleTerrain(registry, capsule, displacement);
		position += displacement * terrain.fraction;

		// 水平速度から前転の軸と速さを求め、反発後は新しい進行方向へ回す。
		const auto spin = No::Vector3::UP.Cross(motion.velocity);
		const float speed = spin.Length();
		if (speed > 1e-4f) {
			No::Quaternion rotation;
			const auto axis = inverseParentRotation.RotateVector(spin / speed);
			rotation.FromAxisAngle(axis, speed * step / std::max(motion.collider.radius, 0.1f));
			transform.rotation = rotation * transform.rotation;
			transform.rotation.Normalize();
		}

		if (terrain.hit) Bounce(motion, terrain.normal);
		auto platformNormal = No::Vector3::ZERO;
		ResolvePlatformContact(registry, motion.collider, position, platformNormal);
		Bounce(motion, platformNormal);
	}
	transform.SetWorldPosition(registry, position);
	// 崖から地面のない場所に落ちた場合も残り続けないようにする。
	if (position.y < motion.launchHeight - kMaxFallDistance) registry.DestroyEntity(enemy);
}

class PlayerAttackHitSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override {
		const auto hits = registry.PollAllEvents<PlayerAttackHitEvent>();
		if (deltaTime <= 0.0f) return;

		for (auto enemy : registry.View<EnemyDefeatMotionComponent, No::TransformComponent>()) {
			UpdateDefeatMotion(registry, enemy, deltaTime);
		}

		for (auto camera : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
			if (registry.Has<CameraLockTag>(camera)) return;
		}
		for (const auto& hit : hits) {
			const auto player = hit.player;
			const auto* action = registry.GetComponent<PlayerActionComponent>(player);
			if (!action || action->activeAction != PlayerActionType::kSpinAttack) continue;
			if (registry.Has<DialogueLockTag>(player) || registry.Has<GoalDirectionLockTag>(player)) continue;
			const auto* death = registry.GetComponent<PlayerDeathSequenceComponent>(player);
			if (death && death->phase != PlayerDeathSequenceComponent::Phase::kBlackOut
				&& death->phase != PlayerDeathSequenceComponent::Phase::kFadeIn) continue;

			auto* layer = registry.GetComponent<CollisionLayerComponent>(hit.enemy);
			if (!layer || (layer->layer & CollisionLayerComponent::Enemy) == CollisionLayerComponent::None) continue;
			const auto* playerTransform = registry.GetComponent<No::TransformComponent>(player);
			const auto* enemyTransform = registry.GetComponent<No::TransformComponent>(hit.enemy);
			const auto* enemyCollider = registry.GetComponent<No::CapsuleCollider>(hit.enemy);
			if (!playerTransform || !enemyTransform || !enemyCollider) continue;

			const auto startPosition = enemyTransform->GetWorldPosition(registry);
			auto away = startPosition - playerTransform->GetWorldPosition(registry);
			away.y = 0.0f;
			if (away.LengthSquared() <= 1e-6f) {
				away = playerTransform->rotation.RotateVector(No::Vector3::FORWARD);
				away.y = 0.0f;
			}
			const auto launchVelocity = away.Normalize() * kLaunchHorizontalSpeed + No::Vector3::UP * kLaunchUpSpeed;
			*registry.AddComponent<EnemyDefeatMotionComponent>(hit.enemy) = {*enemyCollider, launchVelocity, startPosition.y};

			// 撃破時点で本体の接触と通常移動を止め、保存した形状を吹き飛びの地面判定に使う。
			layer->layer = CollisionLayerComponent::None;
			registry.RemoveComponent<No::CapsuleCollider>(hit.enemy);
			registry.RemoveComponent<No::VelocityComponent>(hit.enemy);
		}
	}
};
}

REGISTER_SYSTEM(PlayerAttackHitSystem, "PlayerAttackHitSystem", "Gameplay")
