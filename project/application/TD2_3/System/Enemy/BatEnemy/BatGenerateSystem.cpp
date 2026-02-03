#include "BatGenerateSystem.h"
#include "application/TD2_3/Random/RandomFanc.h"

#include "../../../Component/ColliderComponent.h"
#include "../../../Component/Enemy/BatComponent.h"
#include "../../../Component/PlayerstatusComponent.h"
#include "../../../tag.h"

namespace {
float sGenerateTime = 10.f;
}

void BatGenerateSystem::Update(No::Registry& registry, float deltaTime) {
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

    auto batView = registry.View<BatTag>();
    auto greenBatView = registry.View<BatGreenTag>();

    if (batView.NoEntity() && greenBatView.NoEntity()) {
        timer_ += sGenerateTime;
    }

	if (timer_ > sGenerateTime) {
		timer_ = 0.f;
        sGenerateTime = RNG::GetRandomVal(5.f, 10.f);

        uint32_t generateCount = 0;

        if (RNG::GetRandomValNormalized() > 0.8f) {
            generateCount = 3;
        } else {
            if (RNG::GetRandomValNormalized() > 0.5f) {
                generateCount = 2;
            } else {
                generateCount = 1;
            }
            
        }

        for (uint32_t i = 0; i < generateCount; i++) {



            if (RNG::GetRandomValNormalized() > 0.8f) {
                isGreen_ = true;
            } else {
                isGreen_ = false;
            }


            No::Entity entity = registry.GenerateEntity();

            registry.AddComponent<DeathFlag>(entity);



            auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
            collider->colliderType = ColliderMask::kEnemy;
            collider->collideMask = ColliderMask::kBall;

            auto* transform = registry.AddComponent<No::TransformComponent>(entity);
            transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f);
            transform->translate = GenerateRandomPointInCircle(2.0f, 3.0f);
            transform->scale = 0.f;

            auto* model = registry.AddComponent<No::MeshComponent>(entity);
            auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
            auto m = registry.AddComponent<No::MaterialComponent>(entity);
            registry.AddComponent<EnemyTag>(entity);
            if (isGreen_) {
                registry.AddComponent<BatGreenTag>(entity);
                NoEngine::ModelLoader::LoadModel(resource_.greenName, resource_.greenModelPath, model, animationComp);
                m->materials = NoEngine::ModelLoader::GetMaterial(resource_.greenName);

            } else {
                registry.AddComponent<BatTag>(entity);
                NoEngine::ModelLoader::LoadModel(resource_.modelName, resource_.modelPath, model, animationComp);
                m->materials = NoEngine::ModelLoader::GetMaterial(resource_.modelName);
            }




            auto* enemy = registry.AddComponent<BatComponent>(entity);
            enemy->defaultTranslate = transform->translate;


            m->psoName = L"Renderer : Default PSO";
            m->psoId = NoEngine::Render::GetPSOID(m->psoName);
            m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
            m->drawOutline = true;
        }
	}
}

NoEngine::Vector3 BatGenerateSystem::GenerateRandomPointInCircle(float minRadius, float maxRadius) {
    // 0〜1の乱数を生成して√で均等分布に
    float raw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float r = std::sqrt(raw) * (maxRadius - minRadius) + minRadius;

    // 0〜2πの角度をランダムに生成
    float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;

    float x = r * std::cos(angle);
    float y = r * std::sin(angle);

    return NoEngine::Vector3{ x, y, 0.0f }; // Zは0で平面上に配置
}
