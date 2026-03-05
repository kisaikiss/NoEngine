#include "CarrotControlSystem.h"
#include "application/TD2_3/tag.h"
#include "application/TD2_3/Component/Enemy/VegetableComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/Random/RandomFanc.h"
#include "application/TD2_3/Component/DynamicSplinePath.h"
#include "application/TD2_3/Component/Enemy/EnemyBulletComponent.h"

void CarrotControlSystem::Update(No::Registry& registry, float deltaTime) {
	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	auto view = registry.View<CarrotTag>();
	bool hasEntities = (view.begin() != view.end());
	if (!hasEntities) return;

	for (auto entity : view) {
		auto* death = registry.GetComponent<DeathFlag>(entity);
		if (death->isDead) continue;
		auto* bat = registry.GetComponent<VegetableComponent>(entity);
		VegetableState state = bat->state;


		switch (state) {
		case VegetableState::GENERATE:
			GenerateUpdate(entity, registry, deltaTime);
			break;
		case VegetableState::LIVE:
			LiveUpdate(entity, registry, deltaTime);
			break;
		case VegetableState::DEAD:
			DeadUpdate(entity, registry, deltaTime);
			break;
		}
	}
}

void CarrotControlSystem::LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
	auto* bat = registry.GetComponent<VegetableComponent>(entity);
	bat->t += deltaTime;
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	transform->rotation.FromAxisAngle(No::Vector3::FORWARD, (3.14f + bat->t));
	using namespace NoEngine;

	// 移動
	auto* path = registry.GetComponent<PathComponent>(entity);
	path->t += deltaTime * path->speed;
	if (path->t >= 1.0f) {
		path->t -= 1.0f;

		// 古い制御点を削除
		path->controlPoints.pop_front();

		// 新しい制御点を追加
		const float kMinLength = 2.f;
		path->controlPoints.push_back(GetRandomPosition(path->controlPoints.back(), kMinLength));
	}

	if (path->controlPoints.size() < 4)return;
	transform->translate = CatmullRom(path->controlPoints[0],
		path->controlPoints[1],
		path->controlPoints[2],
		path->controlPoints[3],
		path->t);

	CheckCollideEntity(registry, entity);

	const float kShootTime = 0.8f;
	if (bat->shootTimer > kShootTime) {
		if (bat->shootState == VegetableShootState::NONE) {
			bat->shootState = VegetableShootState::STANBY;
			bat->shootTimer = 0.f;
		}

	}

	if (bat->shootState != VegetableShootState::NONE) {
		VegetableShootState state = bat->shootState;
		bat->shootTimer += 3.f * deltaTime;
		switch (state) {
		case VegetableShootState::STANBY:
			transform->scale = No::Lerp(transform->scale, No::Vector3(1.25f, 1.25f, 1.25f), bat->shootTimer);
			if (bat->shootTimer > 1.f) {
				bat->shootState = VegetableShootState::SHOOT;
				bat->shootTimer = 0.f;
				Shoot(registry, transform);
			}
			break;
		case VegetableShootState::SHOOT:
			transform->scale = No::Lerp(transform->scale, No::Vector3(1.f, 1.f, 1.f), bat->shootTimer);
			if (bat->shootTimer > 1.f) {
				bat->shootState = VegetableShootState::NONE;
				bat->shootTimer = 0.f;
				transform->scale = 1.f;

			}
			break;
		}
	} else {
		bat->shootTimer += deltaTime;
	}
}

void CarrotControlSystem::Shoot(No::Registry& registry, No::TransformComponent* enemyTransform) {
	using namespace NoEngine;
	No::Entity entity = registry.GenerateEntity();
	auto* ultrasound = registry.AddComponent<EnemyBulletComponent>(entity);
	registry.AddComponent<DeathFlag>(entity);

	const float kUltrasoundSpeed = 3.5f;
	No::Vector3 localSpeed = No::Vector3(0.f, -1.f, 0.f);
	ultrasound->velocity = enemyTransform->rotation.RotateVector(localSpeed);
	ultrasound->velocity = ultrasound->velocity.Normalize() * kUltrasoundSpeed;

	auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
	collider->colliderType = ColliderMask::kEnemy;
	collider->collideMask = ColliderMask::kBall;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->rotation = enemyTransform->rotation;
	transform->translate = enemyTransform->translate;


	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
	No::ModelLoader::LoadModel("flame2", "resources/game/td_2304/Model/flame2/flame2.obj", model, animationComp);

	auto m = registry.AddComponent<No::MaterialComponent>(entity);
	m->materials = No::ModelLoader::GetMaterial("flame2");
	m->psoName = L"Renderer : Toon PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
	m->color = No::Color::RED;



	const uint32_t kSmokeNum = 5;
	for (uint32_t i = 0; i < kSmokeNum; i++) {
		GenerateSmokeEffect(registry, transform->translate);
	}
}
