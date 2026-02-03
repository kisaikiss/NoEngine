#include "BossGenerateSystem.h"
#include "application/TD2_3/tag.h"
#include "application/TD2_3/Component/BatBossComponent.h"
#include "application/TD2_3/Component/ColliderComponent.h"
#include "application/TD2_3/Component/DynamicSplinePath.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"

namespace {
const float skGenerateTime = 60.f;
}


void BossGenerateSystem::Update(No::Registry& registry, float deltaTime) {
	if (isGenerate_) return;

	auto playerStatusView = registry.View<PlayerStatusComponent>();
	PlayerStatusComponent* playerStatus = nullptr;
	for (auto playerEntity : playerStatusView) {
		playerStatus = registry.GetComponent<PlayerStatusComponent>(playerEntity);
		if (playerStatus->pendingUpgrade) {
			// レベルアップ選択中はうごかさない
			return;
		}
	}

	timer_ += deltaTime;
	if (timer_ > skGenerateTime) {
		GenerateBatBoss(registry);
		isGenerate_ = true;
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
	NoEngine::ModelLoader::GetModel("batBoss", model, animationComp);

	auto m = registry.AddComponent<No::MaterialComponent>(bossEntity);
	m->materials = NoEngine::ModelLoader::GetMaterial("batBoss");
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
