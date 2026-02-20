#include "WhiteRadishControlSystem.h"
#include "application/TD2_3/tag.h"
#include "application/TD2_3/Component/Enemy/VegetableComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/Random/RandomFanc.h"
#include "application/TD2_3/Component/DynamicSplinePath.h"

void WhiteRadishControlSystem::Update(No::Registry& registry, float deltaTime) {
	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	auto view = registry.View<WhiteRadishTag>();
	if (view.Empty())return;

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

void WhiteRadishControlSystem::GenerateUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
	auto* bat = registry.GetComponent<VegetableComponent>(entity);
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);

	CheckCollideEntity(registry, entity);

	bat->t += 3.f * deltaTime;
	if (bat->t > 1.f) bat->t = 1.f;
	transform->scale = NoEngine::Easing::EaseOutCirc<No::Vector3>(No::Vector3(0.f, 0.f, 0.f), No::Vector3(1.f, 1.f, 1.f), bat->t);
	transform->rotation.FromAxisAngle(No::Vector3::FORWARD, (3.14f + bat->t * 5.f));
	if (transform->scale.x >= 1.f) {
		transform->scale = 1.f;
		bat->state = VegetableState::LIVE;
		bat->t = 0.f;
		transform->rotation.FromAxisAngle(No::Vector3::FORWARD, 0.f);
	}


	if (!bat->isStarted) {
		const uint32_t kSmokeNum = 10;
		for (uint32_t i = 0; i < kSmokeNum; i++) {
			GenerateSmokeEffect(registry, transform->translate);
		}
		bat->isStarted = true;

	}

}

void WhiteRadishControlSystem::LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
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
}

void WhiteRadishControlSystem::DeadUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	auto* bat = registry.GetComponent<VegetableComponent>(entity);
	auto* death = registry.GetComponent<DeathFlag>(entity);
	bat->t += 1.5f * deltaTime;
	transform->scale = NoEngine::Easing::EaseInOutBack<No::Vector3>(No::Vector3(1.f, 1.f, 1.f), No::Vector3(0.f, 0.f, 0.f), bat->t);
	transform->rotation.FromAxisAngle(No::Vector3::FORWARD, (3.14f + bat->t * 25.f));
	if (transform->scale.x <= 0) {
		auto view = registry.View<PlayerStatusComponent>();
		for (auto playerEntity : view) {
			auto* status = registry.GetComponent<PlayerStatusComponent>(playerEntity);
			status->score += int32_t(150.0f * status->scoreRatio);
			status->exp++;
			status->scoreRatio += 0.1f;
		}
		death->isDead = true;
		const uint32_t kSmokeNum = 10;
		for (uint32_t i = 0; i < kSmokeNum; i++) {
			GenerateSmokeEffect(registry, transform->translate);
		}
	}

}

void WhiteRadishControlSystem::GenerateSmokeEffect(No::Registry& registry, No::Vector3 position) {
	auto smoke = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::TransformComponent>(smoke);
	t->translate = position;
	auto* model = registry.AddComponent<No::MeshComponent>(smoke);
	auto* material = registry.AddComponent<No::MaterialComponent>(smoke);
	registry.AddComponent<DeathFlag>(smoke);
	No::ModelLoader::LoadModel("smoke", "resources/game/td_2304/Model/effect/smoke.gltf", model);
	material->materials = No::ModelLoader::GetMaterial("smoke");
	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
	auto* smokeComp = registry.AddComponent<SmokeEffectComponent>(smoke);
	smokeComp->velocity = RNG::GetRandomVector3(-2.f, 2.f);

}

void WhiteRadishControlSystem::CheckCollideEntity(No::Registry& registry, No::Entity enemyEntity) {
	auto* bat = registry.GetComponent<VegetableComponent>(enemyEntity);
	auto* collider = registry.GetComponent<SphereColliderComponent>(enemyEntity);

	if (collider->isCollied) {
		bat->t = 0.f;
		bat->state = VegetableState::DEAD;
	}
}
