#include "BossControlSystem.h"
#include "../../../Component/ColliderComponent.h"
#include"../../../Component/BatBossComponent.h"
#include"../../../Component/DynamicSplinePath.h"
#include "../../../tag.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Math/Types/Calculations/QuaternionCalculations.h"
#include "../EnemyCommonMove.h"
#include "application/TD2_3/Component/Enemy/EnemyBulletComponent.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/Random/RandomFanc.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"

void BossControlSystem::Update(No::Registry& registry, float deltaTime) {

	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	auto view = registry.View<
		No::TransformComponent,
		No::MaterialComponent,
		SphereColliderComponent,
		BatBossComponent,
		Boss1Tag>();

	for (auto entity : view) {

		auto* material = registry.GetComponent<No::MaterialComponent>(entity);
		auto* collider = registry.GetComponent<SphereColliderComponent>(entity);
		auto* boss = registry.GetComponent<BatBossComponent>(entity);
		BatBossState state = boss->state;
		const float kInvincibleTime = 0.5f;
		switch (state) {
		case BatBossState::GENERATE:
			GenerateUpdate(registry, entity, deltaTime);
			break;
		case BatBossState::MOVE:
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
				boss->state = BatBossState::DEAD;
				boss->shootTimer = 0.f;
				auto* path = registry.GetComponent<PathComponent>(entity);
				path->speed = 1.f;
				No::SoundEffectPlay("batDie", 1.f);
			}
			break;
		case BatBossState::DEAD:
			DeadUpdate(registry, entity, deltaTime);
			break;
		}




	}

}

void BossControlSystem::GenerateUpdate(No::Registry& registry, No::Entity entity, float deltaTime) {
	auto* bat = registry.GetComponent<BatBossComponent>(entity);
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);

	bat->t += 1.f * deltaTime;
	if (bat->t > 1.f) bat->t = 1.f;
	transform->scale = NoEngine::Easing::EaseOutCirc<NoEngine::Vector3>(NoEngine::Vector3(0.f, 0.f, 0.f), NoEngine::Vector3(1.f, 1.f, 1.f), bat->t);

	if (transform->scale.x >= 1.f) {
		transform->scale = 1.f;
		bat->state = BatBossState::MOVE;
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

void BossControlSystem::MoveUpdate(No::Registry& registry, No::Entity entity, float deltaTime) {
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


	// 玉の方を見る
	float finalLength = 100000.f;
	No::Entity ballEn = 0;
	auto view = registry.View<BallTag>();
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

	// 超音波を吐く
	auto* bat = registry.GetComponent<BatBossComponent>(entity);
	bat->t += deltaTime;
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
			transform->scale = No::Lerp(transform->scale, NoEngine::Vector3(1.25f, 1.25f, 1.25f), bat->shootTimer);
			if (bat->shootTimer > 1.f) {
				bat->shootState = BatShootState::SHOOT;
				bat->shootTimer = 0.f;
				Shoot(registry, transform, t->GetWorldPosition());
			}
			break;
		case BatShootState::SHOOT:
			transform->scale = No::Lerp(transform->scale, NoEngine::Vector3(1.f, 1.f, 1.f), bat->shootTimer);
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

void BossControlSystem::DeadUpdate(No::Registry& registry, No::Entity entity, float deltaTime) {
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

	auto* bat = registry.GetComponent<BatBossComponent>(entity);
	bat->t += 1.5f * deltaTime;
	bat->shootTimer += 6.f * deltaTime;
	bat->deadTimer += 0.4f * deltaTime;
	transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, (3.14f + bat->t * 25.f));
	transform->scale = NoEngine::Easing::EaseInOutBack<NoEngine::Vector3>(NoEngine::Vector3(1.f, 1.f, 1.f), NoEngine::Vector3(0.f, 0.f, 0.f), bat->deadTimer);
	if (bat->deadTimer > 1.f) {
		auto* death = registry.GetComponent<DeathFlag>(entity);
		death->isDead = true;
		auto view = registry.View<PlayerStatusComponent>();
		for (auto playerEntity : view) {
			auto* status = registry.GetComponent<PlayerStatusComponent>(playerEntity);
			status->score += 1000;
			status->exp += 30;

		}
	}

	if (bat->shootTimer > 1.f && transform->scale.x > 0.5f) {
		bat->shootTimer = 0.f;
		const float kSmokeNum = 3;
		for (uint32_t i = 0; i < kSmokeNum; i++) {
			Vector3 smokePosition = transform->GetWorldPosition() + Vector3(RNG::GetRandomValNormalized(), RNG::GetRandomValNormalized(), -1.f);
			GenerateSmokeEffect(registry, smokePosition);
		}
	}


}



void BossControlSystem::Shoot(No::Registry& registry, No::TransformComponent* enemyTransform, const NoEngine::Vector3& target) {
	using namespace NoEngine;
	No::Entity entity = registry.GenerateEntity();
	auto* ultrasound = registry.AddComponent<EnemyBulletComponent>(entity);
	registry.AddComponent<DeathFlag>(entity);

	const float kUltrasoundSpeed = 5.f;
	ultrasound->velocity = target - enemyTransform->translate;
	ultrasound->velocity = ultrasound->velocity.Normalize() * kUltrasoundSpeed;

	auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
	collider->colliderType = ColliderMask::kEnemy;
	collider->collideMask = ColliderMask::kBall;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);
	transform->translate = enemyTransform->translate;
	transform->scale = 2.5f;
	Vector3 offset = { 0.f,0.f,-5.f };
	LookTarget(*transform, target + offset);


	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
	NoEngine::ModelLoader::LoadModel("wave", "resources/game/td_2304/Model/wave/wave.obj", model, animationComp);

	auto m = registry.AddComponent<No::MaterialComponent>(entity);
	m->materials = NoEngine::ModelLoader::GetMaterial("wave");
	m->psoName = L"Renderer : Toon PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
	m->color = Color::RED;

	const uint32_t kSmokeNum = 5;
	for (uint32_t i = 0; i < kSmokeNum; i++) {
		GenerateSmokeEffect(registry, transform->translate);
	}

}


void BossControlSystem::GenerateSmokeEffect(No::Registry& registry, NoEngine::Vector3 position) {
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

