#include "BatGreenControlSystem.h"
#include "application/TD2_3/tag.h"
#include "application/TD2_3/Component/Enemy/BatComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/Random/RandomFanc.h"
#include "application/TD2_3/Component/Enemy/EnemyBulletComponent.h"
#include "../EnemyCommonMove.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "engine/Math/Easing.h"

using namespace No;

void BatGreenControlSystem::Update(No::Registry& registry, float deltaTime) {

	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	auto view = registry.View<BatGreenTag>();

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
	transform->rotation = Slerp(transform->rotation, tt.rotation, 7.f * deltaTime);

	const float kShootTime = 5.f;
	if (bat->shootTimer > kShootTime) {
		if (bat->shootState == BatShootState::NONE) {
			bat->shootState = BatShootState::STANBY;
			bat->shootTimer = 0.f;
		}
		
	}

	if (bat->shootState != BatShootState::NONE) {
		BatShootState state = bat->shootState;
		bat->shootTimer += 3.f * deltaTime;
		switch (state) {
		case BatShootState::STANBY:
			transform->scale = No::Lerp(transform->scale, No::Vector3(1.25f, 1.25f, 1.25f), bat->shootTimer);
			if (bat->shootTimer > 1.f) {
				bat->shootState = BatShootState::SHOOT;
				bat->shootTimer = 0.f;
				Shoot(registry, transform, t->GetWorldPosition());
			}
			break;
		case BatShootState::SHOOT:
			transform->scale = No::Lerp(transform->scale, No::Vector3(1.f, 1.f, 1.f), bat->shootTimer);
			if (bat->shootTimer > 1.f) {
				bat->shootState = BatShootState::NONE;
				bat->shootTimer = 0.f;
				transform->scale = 1.f;
				
			}
			break;
		}
	} else {
		bat->shootTimer += deltaTime;
	}

}

void BatGreenControlSystem::Shoot(No::Registry& registry, No::TransformComponent* enemyTransform, const No::Vector3& target) {
	using namespace NoEngine;
	No::Entity entity = registry.GenerateEntity();
	auto* ultrasound = registry.AddComponent<EnemyBulletComponent>(entity);
	registry.AddComponent<DeathFlag>(entity);

	const float kUltrasoundSpeed = 3.5f;
	ultrasound->velocity = target - enemyTransform->translate;
	ultrasound->velocity = ultrasound->velocity.Normalize() * kUltrasoundSpeed;

	auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
	collider->colliderType = ColliderMask::kEnemy;
	collider->collideMask = ColliderMask::kBall;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);
	transform->translate = enemyTransform->translate;
	Vector3 offset = { 0.f,0.f,-5.f };
	LookTarget(*transform, target + offset);


	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
	No::ModelLoader::LoadModel("wave", "resources/game/td_2304/Model/wave/wave.obj", model, animationComp);

	auto m = registry.AddComponent<No::MaterialComponent>(entity);
	m->materials = No::ModelLoader::GetMaterial("wave");
	m->psoName = L"Renderer : Toon PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
	m->color = Color::RED;

	const uint32_t kSmokeNum = 5;
	for (uint32_t i = 0; i < kSmokeNum; i++) {
		GenerateSmokeEffect(registry, transform->translate);
	}

}
