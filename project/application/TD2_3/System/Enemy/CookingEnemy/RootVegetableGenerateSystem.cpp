#include "RootVegetableGenerateSystem.h"
#include "application/TD2_3/Random/RandomFanc.h"

#include "../../../Component/ColliderComponent.h"
#include "../../../Component/Enemy/VegetableComponent.h"
#include "../../../Component/PlayerstatusComponent.h"
#include "../../../tag.h"
#include "../../../Component/PhaseComponent.h"
#include "application/TD2_3/Component/DynamicSplinePath.h"

void RootVegetableGenerateSystem::Update(No::Registry& registry, float deltaTime) {
    auto phaseView = registry.View<PhaseComponent>();
    for (auto entity : phaseView) {
        auto* phase = registry.GetComponent<PhaseComponent>(entity);
        if (phase->phase != Phase::TWO) {
            // Phase2以外では動かさない
            return;
        }
    }
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

    auto carrotView = registry.View<CarrotTag>();
    auto whiteView = registry.View<WhiteRadishTag>();

    if (carrotView.NoEntity() && whiteView.NoEntity()) {
        timer_ += generateTime_;
    }

    if (timer_ > generateTime_) {
        timer_ = 0.f;
        generateTime_ = RNG::GetRandomVal(3.f, 7.f);

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



            if (RNG::GetRandomValNormalized() > 0.5f) {
                isCarrot_ = true;
            } else {
                isCarrot_ = false;
            }


            No::Entity entity = registry.GenerateEntity();

            registry.AddComponent<DeathFlag>(entity);



            auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
            collider->colliderType = ColliderMask::kEnemy;
            collider->collideMask = ColliderMask::kBall;

            auto* transform = registry.AddComponent<No::TransformComponent>(entity);
            transform->rotation.FromAxisAngle(No::Vector3::UP, 3.14f);
            transform->translate = GenerateRandomPointInCircle(2.0f, 3.0f);
            transform->scale = 0.f;

            auto* model = registry.AddComponent<No::MeshComponent>(entity);
            auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
            auto m = registry.AddComponent<No::MaterialComponent>(entity);
            registry.AddComponent<EnemyTag>(entity);
            if (isCarrot_) {
                registry.AddComponent<CarrotTag>(entity);
                No::ModelLoader::LoadModel("carrot", "resources/game/td_2304/Model/normalVegetable/carrot.obj", model, animationComp);
                m->materials = No::ModelLoader::GetMaterial("carrot");

            } else {
                registry.AddComponent<WhiteRadishTag>(entity);
                No::ModelLoader::LoadModel("white", "resources/game/td_2304/Model/normalVegetable/pot.obj", model, animationComp);
                m->materials = No::ModelLoader::GetMaterial("white");
            }




            auto* enemy = registry.AddComponent<VegetableComponent>(entity);
            enemy->defaultTranslate = transform->translate;


            m->psoName = L"Renderer : Default PSO";
            m->psoId = NoEngine::Render::GetPSOID(m->psoName);
            m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
            m->drawOutline = true;

            auto* path = registry.AddComponent<PathComponent>(entity);

            const float kMinLength = 2.f;
            path->controlPoints.push_back(GetRandomPosition(No::Vector3(), 0.f));
            path->controlPoints.push_back(GetRandomPosition(path->controlPoints[0], kMinLength));
            path->controlPoints.push_back(GetRandomPosition(path->controlPoints[1], kMinLength));
            path->controlPoints.push_back(GetRandomPosition(path->controlPoints[2], kMinLength));

            path->speed = 0.2f;

            transform->translate = path->controlPoints[1];
        }
    }
}


No::Vector3 RootVegetableGenerateSystem::GenerateRandomPointInCircle(float minRadius, float maxRadius) {
    // 0〜1の乱数を生成して√で均等分布に
    float raw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float r = std::sqrt(raw) * (maxRadius - minRadius) + minRadius;

    // 0〜2πの角度をランダムに生成
    float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;

    float x = r * std::cos(angle);
    float y = r * std::sin(angle);

    return No::Vector3{ x, y, 0.0f }; // Zは0で平面上に配置
}
