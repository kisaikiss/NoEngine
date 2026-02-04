#include "PotControlSystem.h"
#include "../../../Component/ColliderComponent.h"
#include"../../../Component/Enemy/PotBossComponent.h"
#include"../../../Component/DynamicSplinePath.h"
#include "../../../tag.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "../EnemyCommonMove.h"
#include "application/TD2_3/Component/Enemy/EnemyBulletComponent.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/Random/RandomFanc.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/PhaseComponent.h"
#include "application/TD2_3/Component/Enemy/IngredientsComponent.h"

void PotControlSystem::Update(No::Registry& registry, float deltaTime) {

	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	auto view = registry.View<Boss2Tag>();
	if (view.NoEntity()) return;

	for (auto entity : view) {

		auto* material = registry.GetComponent<No::MaterialComponent>(entity);
		auto* collider = registry.GetComponent<SphereColliderComponent>(entity);
		auto* boss = registry.GetComponent<PotBossComponent>(entity);
		PotBossState state = boss->state;
		const float kInvincibleTime = 0.5f;
		switch (state) {
		case PotBossState::GENERATE:
			GenerateUpdate(registry, entity, deltaTime);
			break;
		case PotBossState::MOVE:
			MoveUpdate(registry, entity, deltaTime);
			// ボールと当たっていたら
			if (collider->isCollied) {
				boss->hp--;
				if (boss->invincibleTimer == 0.f)
					boss->invincibleTimer = kInvincibleTime;
				material->color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
			} else {
				// 無敵時間中なら
				if (boss->invincibleTimer > 0.f) {
					boss->invincibleTimer -= deltaTime;
				} else {
					boss->invincibleTimer = 0.f;
					material->color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}

			if (boss->hp <= 0) {
				material->color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
				boss->state = PotBossState::DEAD;
				boss->shootTimer = 0.f;
				auto* path = registry.GetComponent<PathComponent>(entity);
				path->speed = 1.f;
				No::SoundEffectPlay("batDie", 1.f);
			}
			break;
		case PotBossState::DEAD:
			DeadUpdate(registry, entity, deltaTime);
			break;
		}




	}

}

void PotControlSystem::GenerateUpdate(No::Registry& registry, No::Entity entity, float deltaTime) {
	auto* bat = registry.GetComponent<PotBossComponent>(entity);
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);

	bat->t += 1.f * deltaTime;
	if (bat->t > 1.f) bat->t = 1.f;
	transform->scale = NoEngine::Easing::EaseOutCirc<NoEngine::Vector3>(NoEngine::Vector3(0.f, 0.f, 0.f), NoEngine::Vector3(1.f, 1.f, 1.f), bat->t);

	if (transform->scale.x >= 1.f) {
		transform->scale = 1.f;
		bat->state = PotBossState::MOVE;
		bat->t = 0.f;
		transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, PI);
	}


	if (!bat->isStarted) {
		const uint32_t kSmokeNum = 30;
		for (uint32_t i = 0; i < kSmokeNum; i++) {
			GenerateSmokeEffect(registry, transform->translate);
		}
		bat->isStarted = true;

	}
}

void PotControlSystem::MoveUpdate(No::Registry& registry, No::Entity entity, float deltaTime) {
	using namespace NoEngine;
	using namespace MathCalculations;

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
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	transform->translate = CatmullRom(path->controlPoints[0],
		path->controlPoints[1],
		path->controlPoints[2],
		path->controlPoints[3],
		path->t);


	// 回転
	auto pot = registry.GetComponent<PotBossComponent>(entity);
	pot->t += deltaTime;
	Quaternion rotate{};
	rotate.FromAxisAngle(Vector3::FORWARD, std::sinf(pot->t));

	Quaternion rotate2{};
	rotate2.FromAxisAngle(Vector3::UP, PI);
	transform->rotation = rotate * rotate2;

	// 具材出す
	
	if (std::sinf(pot->t) > 0.6f) {
		pot->shootTimer += 5.f * deltaTime;
		if (pot->shootTimer > 1.f) {
			pot->shootTimer = 0.f;
			Vector3 velocity = Vector3(RNG::GetRandomVal(-5.f, -2.f), RNG::GetRandomVal(5.f, 10.f), 0.f);
			
			Shoot(registry, transform, velocity);
		}

	}else if(std::sinf(pot->t) < -0.6f) {
		pot->shootTimer += 5.f * deltaTime;
		if (pot->shootTimer > 1.f) {
			pot->shootTimer = 0.f;
			Vector3 velocity = Vector3(RNG::GetRandomVal(2.f, 5.f), RNG::GetRandomVal(5.f, 10.f), 0.f);
			Shoot(registry, transform, velocity);
		}

	}

	
}

void PotControlSystem::DeadUpdate(No::Registry& registry, No::Entity entity, float deltaTime) {
	using namespace NoEngine;
	using namespace MathCalculations;

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
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	transform->translate = CatmullRom(path->controlPoints[0],
		path->controlPoints[1],
		path->controlPoints[2],
		path->controlPoints[3],
		path->t);

	auto* bat = registry.GetComponent<PotBossComponent>(entity);
	bat->t += 1.5f * deltaTime;
	bat->shootTimer += 6.f * deltaTime;
	bat->deadTimer += 0.4f * deltaTime;
	transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, (3.14f + bat->t * 25.f));
	transform->scale = NoEngine::Easing::EaseInOutBack<NoEngine::Vector3>(NoEngine::Vector3(1.f, 1.f, 1.f), NoEngine::Vector3(0.f, 0.f, 0.f), bat->deadTimer);
	if (bat->deadTimer > 1.f) {
		auto* death = registry.GetComponent<DeathFlag>(entity);
		death->isDead = true;
		if (death->isDead)
		{
			registry.EmitEvent(No::SceneChangeEvent("ResultScene"));
		}
		auto view = registry.View<PlayerStatusComponent>();
		for (auto playerEntity : view) {
			auto* status = registry.GetComponent<PlayerStatusComponent>(playerEntity);
			status->score += int32_t(1500.f * status->scoreRatio);
		}

		auto phaseView = registry.View<PhaseComponent>();
		for (auto phaseEntity : phaseView) {
			auto* phase = registry.GetComponent<PhaseComponent>(phaseEntity);
			phase->phase = Phase::TWO;
		}
	}

	if (bat->shootTimer > 1.f && transform->scale.x > 0.5f) {
		bat->shootTimer = 0.f;
		const float kSmokeNum = 3;
		for (uint32_t i = 0; i < kSmokeNum; i++) {
			Vector3 smokePosition = transform->translate + Vector3(RNG::GetRandomValNormalized(), RNG::GetRandomValNormalized(), -1.f);
			GenerateSmokeEffect(registry, smokePosition);
		}
	}


}



void PotControlSystem::Shoot(No::Registry& registry, No::TransformComponent* enemyTransform, NoEngine::Vector3 velocity) {
	using namespace NoEngine;
	No::Entity entity = registry.GenerateEntity();
	auto* ultrasound = registry.AddComponent<IngredientsComponent>(entity);
	registry.AddComponent<DeathFlag>(entity);

	ultrasound->velocity = velocity;

	auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
	collider->colliderType = ColliderMask::kEnemy;
	collider->collideMask = ColliderMask::kBall;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);
	transform->translate = enemyTransform->translate;
	transform->scale = 1.f;
	Vector3 offset = { 0.f,0.f,-5.f };
	

	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
	NoEngine::ModelLoader::LoadModel("veg", "resources/game/td_2304/Model/potVegetable/veg.obj", model, animationComp);

	auto m = registry.AddComponent<No::MaterialComponent>(entity);
	m->materials = NoEngine::ModelLoader::GetMaterial("veg");
	m->psoName = L"Renderer : Toon PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
	m->color = Color(RNG::GetRandomVal(0.f, 1.f), RNG::GetRandomVal(0.f, 1.f), RNG::GetRandomVal(0.f, 1.f), 1.f);

	const uint32_t kSmokeNum = 5;
	for (uint32_t i = 0; i < kSmokeNum; i++) {
		GenerateSmokeEffect(registry, transform->translate);
	}

}


void PotControlSystem::GenerateSmokeEffect(No::Registry& registry, NoEngine::Vector3 position) {
	auto smoke = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::TransformComponent>(smoke);
	t->translate = position;
	auto* model = registry.AddComponent<No::MeshComponent>(smoke);
	auto* material = registry.AddComponent<No::MaterialComponent>(smoke);
	registry.AddComponent<DeathFlag>(smoke);
	NoEngine::ModelLoader::LoadModel("smoke", "resources/game/td_2304/Model/effect/smoke.gltf", model);
	material->materials = NoEngine::ModelLoader::GetMaterial("smoke");
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
	auto* smokeComp = registry.AddComponent<SmokeEffectComponent>(smoke);
	smokeComp->velocity = RNG::GetRandomVector3(-2.f, 2.f);

}

