#include "BatGreenControlSystem.h"
#include "application/TD2_3/tag.h"
#include "application/TD2_3/Component/Enemy/BatComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/Random/RandomFanc.h"
#include "../EnemyCommonMove.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"

void BatGreenControlSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<BatGreenTag>();
	if (view.Empty()) return;
	for (auto entity : view) {
		auto* death = registry.GetComponent<DeathFlag>(entity);
		if (death->isDead) continue;
		auto* bat = registry.GetComponent<BatComponent>(entity);
		BatState state = bat->state;


		switch (state) {
		case BatState::GENERATE:
			GenerateUpdate(entity, registry, deltaTime);
			break;
		case BatState::LIVE:
			LiveUpdate(entity, registry, deltaTime);
			break;
		case BatState::DEAD:
			DeadUpdate(entity, registry, deltaTime);
			break;
		}
	}
}

void BatGreenControlSystem::LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
	auto view = registry.View<BallTag>();
	(void)deltaTime;
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	auto* bat = registry.GetComponent<BatComponent>(entity);
	using namespace NoEngine;
	using namespace MathCalculations;
	bat->t += deltaTime;
	transform->translate.y = bat->defaultTranslate.y + std::sinf(bat->t) / 2.f;
	CheckCollideEntity(registry, entity);
	

	float finalLength = 100000.f;
	No::Entity ballEn = 0;
	for (auto e : view) {
		auto* t = registry.GetComponent<No::TransformComponent>(e);
		
		float length = LengthSquared(transform->GetWorldPosition() - t->GetWorldPosition());

		if (length < finalLength) {
			finalLength = length;
			ballEn = e;
		}	
	}
	if (ballEn == 0) return;
	auto* t = registry.GetComponent<No::TransformComponent>(ballEn);
	Vector3 offset = { 0.f,0.f,-10.f };
	No::TransformComponent tt = *transform;
	LookTarget(tt, t->GetWorldPosition() + offset);
	transform->rotation = Slerp(transform->rotation, tt.rotation, 0.1f);



}

void BatGreenControlSystem::Shoot(No::Registry& registry) {
	(void)registry;
}
