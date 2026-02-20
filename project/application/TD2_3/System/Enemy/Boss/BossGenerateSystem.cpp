#include "BossGenerateSystem.h"
#include "application/TD2_3/tag.h"
#include "application/TD2_3/Component/Enemy/PotBossComponent.h"
#include "application/TD2_3/Component/BatBossComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/DynamicSplinePath.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/PhaseComponent.h"

namespace {
const float skGenerateTime = 60.f;
}

using namespace No;
void BossGenerateSystem::Update(No::Registry& registry, float deltaTime) {

	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	auto phaseView = registry.View<PhaseComponent>();
	PhaseComponent* phaseComp = nullptr;

	for (auto entity : phaseView) {
		phaseComp = registry.GetComponent<PhaseComponent>(entity);
	}

	if (phaseComp == nullptr) {
		return;
	}

	Phase phase = phaseComp->phase;
	playerStatus->currentPhase = phase;

	switch (phase) {
	case Phase::ONE:
		if (!isGenerate_) {
			timer_ += deltaTime;
		}
		break;
	case Phase::TWO:
		if (!isPotGenerate_) {
			timer_ += deltaTime;
		}
		break;
	}

	if (timer_ > skGenerateTime) {
		switch (phase) {
		case Phase::ONE:
			if (!isGenerate_) {
				GenerateBatBoss(registry);
				isGenerate_ = true;
				timer_ = 0.f;
			}
			break;
		case Phase::TWO:
			if (!isPotGenerate_) {
				GeneratePotBoss(registry);
				isPotGenerate_ = true;
			}
			break;
		}
	}
}

void BossGenerateSystem::GenerateBatBoss(No::Registry& registry) {
	using namespace NoEngine;

	No::Entity bossEntity = registry.GenerateEntity();
	registry.AddComponent<Boss1Tag>(bossEntity);
	registry.AddComponent<EnemyTag>(bossEntity);
	registry.AddComponent<DeathFlag>(bossEntity);
	registry.AddComponent<BatBossComponent>(bossEntity);
	auto* collider = registry.AddComponent<SphereColliderComponent>(bossEntity);
	collider->colliderType = ColliderMask::kEnemy;
	collider->collideMask = ColliderMask::kBall;

	auto* transform = registry.AddComponent<No::TransformComponent>(bossEntity);
	transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);


	auto* model = registry.AddComponent<No::MeshComponent>(bossEntity);
	auto* animationComp = registry.AddComponent<No::AnimatorComponent>(bossEntity);
	No::ModelLoader::GetModel("batBoss", model, animationComp);

	auto m = registry.AddComponent<No::MaterialComponent>(bossEntity);
	m->materials = No::ModelLoader::GetMaterial("batBoss");
	m->drawOutline = true;
	m->psoName = L"Renderer : DefaultSkinned PSO";
	m->enableSkinning = true;
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

	auto* path = registry.AddComponent<PathComponent>(bossEntity);

	const float kMinLength = 2.f;
	path->controlPoints.push_back(GetRandomPosition(Vector3(), 0.f));
	path->controlPoints.push_back(GetRandomPosition(path->controlPoints[0], kMinLength));
	path->controlPoints.push_back(GetRandomPosition(path->controlPoints[1], kMinLength));
	path->controlPoints.push_back(GetRandomPosition(path->controlPoints[2], kMinLength));

	path->speed = 0.3f;

	transform->translate = path->controlPoints[1];
}

void BossGenerateSystem::GeneratePotBoss(No::Registry& registry) {
	using namespace NoEngine;

	No::Entity bossEntity = registry.GenerateEntity();
	registry.AddComponent<Boss2Tag>(bossEntity);
	registry.AddComponent<EnemyTag>(bossEntity);
	registry.AddComponent<DeathFlag>(bossEntity);
	registry.AddComponent<PotBossComponent>(bossEntity);
	auto* collider = registry.AddComponent<SphereColliderComponent>(bossEntity);
	collider->colliderType = ColliderMask::kEnemy;
	collider->collideMask = ColliderMask::kBall;

	auto* transform = registry.AddComponent<No::TransformComponent>(bossEntity);
	transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);


	auto* model = registry.AddComponent<No::MeshComponent>(bossEntity);
	No::ModelLoader::GetModel("potBoss", model);

	auto m = registry.AddComponent<No::MaterialComponent>(bossEntity);
	m->materials = No::ModelLoader::GetMaterial("potBoss");
	m->drawOutline = true;
	m->psoName = L"Renderer : Default PSO";
	m->enableSkinning = false;
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

	auto* path = registry.AddComponent<PathComponent>(bossEntity);

	const float kMinLength = 2.f;
	path->controlPoints.push_back(GetRandomPosition(Vector3(), 0.f));
	path->controlPoints.push_back(GetRandomPosition(path->controlPoints[0], kMinLength));
	path->controlPoints.push_back(GetRandomPosition(path->controlPoints[1], kMinLength));
	path->controlPoints.push_back(GetRandomPosition(path->controlPoints[2], kMinLength));

	path->speed = 0.3f;

	transform->translate = path->controlPoints[1];
}
