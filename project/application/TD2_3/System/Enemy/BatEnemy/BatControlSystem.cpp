#include "BatControlSystem.h"
#include "application/TD2_3/tag.h"
#include "application/TD2_3/Component/Enemy/BatComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/Random/RandomFanc.h"

void BatControlSystem::Update(No::Registry& registry, float deltaTime) {
	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	auto view = registry.View<BatTag>();
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

#ifdef USE_IMGUI
	auto StatusView = registry.View<PlayerStatusComponent>();
	for (auto playerEntity : StatusView) {
		auto* status = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (No::Keyboard::IsTrigger('I')) {
			if (isDebug_) {
				isDebug_ = false;
			} else {
				isDebug_ = true;
			}
		}
		if (isDebug_) {
			
			status->hp = status->hpMax;
			
		}
	}
#endif // USE_IMGUI
}

void BatControlSystem::GenerateUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
	auto* bat = registry.GetComponent<BatComponent>(entity);
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);

	CheckCollideEntity(registry, entity);

	bat->t += 3.f * deltaTime;
	if (bat->t > 1.f) bat->t = 1.f;
	transform->scale = NoEngine::Easing::EaseOutCirc<NoEngine::Vector3>(NoEngine::Vector3(0.f, 0.f, 0.f), NoEngine::Vector3(1.f, 1.f, 1.f), bat->t);
	transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, (3.14f + bat->t * 5.f));
	if (transform->scale.x >= 1.f) {
		transform->scale = 1.f;
		bat->state = BatState::LIVE;
		bat->t = 0.f;
		transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, PI);
	}
	

	if (!bat->isStarted) {
		const uint32_t kSmokeNum = 10;
		for (uint32_t i = 0; i < kSmokeNum; i++) {
			GenerateSmokeEffect(registry, transform->translate);
		}
		bat->isStarted = true;
		
	}
	
}

void BatControlSystem::LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
	auto* bat = registry.GetComponent<BatComponent>(entity);
	bat->t += deltaTime;
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	transform->translate.y = bat->defaultTranslate.y + std::sinf(bat->t);
	CheckCollideEntity(registry, entity);
}

void BatControlSystem::DeadUpdate(No::Entity entity, No::Registry& registry, float deltaTime) {
	auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	auto* bat = registry.GetComponent<BatComponent>(entity);
	auto* death = registry.GetComponent<DeathFlag>(entity);
	bat->t += 1.5f * deltaTime;
	transform->scale = NoEngine::Easing::EaseInOutBack<NoEngine::Vector3>(NoEngine::Vector3(1.f, 1.f, 1.f), NoEngine::Vector3(0.f, 0.f, 0.f), bat->t);
	transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, (3.14f + bat->t * 25.f));
	if (transform->scale.x <= 0) {
		auto view = registry.View<PlayerStatusComponent>();
		for (auto playerEntity : view) {
			auto* status = registry.GetComponent<PlayerStatusComponent>(playerEntity);
			
			status->score += int32_t(100 * status->scoreRatio);
			status->exp++;

		}
		death->isDead = true;
		const uint32_t kSmokeNum = 10;
		for (uint32_t i = 0; i < kSmokeNum; i++) {
			GenerateSmokeEffect(registry, transform->translate);
		}
	}
	
}

void BatControlSystem::GenerateSmokeEffect(No::Registry& registry, NoEngine::Vector3 position) {
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

void BatControlSystem::CheckCollideEntity(No::Registry& registry, No::Entity enemyEntity) {
	auto* bat = registry.GetComponent<BatComponent>(enemyEntity);
	auto* collider = registry.GetComponent<SphereColliderComponent>(enemyEntity);

	if (collider->isCollied) {
		bat->t = 0.f;
		bat->state = BatState::DEAD;
	}
}
