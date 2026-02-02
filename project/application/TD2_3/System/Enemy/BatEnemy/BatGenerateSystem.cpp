#include "BatGenerateSystem.h"
#include "application/TD2_3/Random/RandomFanc.h"

#include "../../../Component/ColliderComponent.h"
#include "../../../Component/Enemy/BatComponent.h"

#include "../../../tag.h"

namespace {
float sGenerateTime = 10.f;
}

void BatGenerateSystem::Update(No::Registry& registry, float deltaTime) {
	timer_ += deltaTime;
	if (timer_ > sGenerateTime) {
		timer_ = 0.f;
        sGenerateTime = RNG::GetRandomVal(1.f, 4.f);

        No::Entity entity = registry.GenerateEntity();
        registry.AddComponent<BatTag>(entity);
        registry.AddComponent<DeathFlag>(entity);

        auto* enemy = registry.AddComponent<BatComponent>(entity);

        auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
        collider->colliderType = ColliderMask::kEnemy;
        collider->collideMask = ColliderMask::kBall;

        auto* transform = registry.AddComponent<No::TransformComponent>(entity);
        transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f);
        transform->translate = GenerateRandomPointInCircle(2.0f, 3.0f);
        transform->scale = 0.f;

        enemy->defaultTranslate = transform->translate;

        auto* model = registry.AddComponent<No::MeshComponent>(entity);
        auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
        NoEngine::ModelLoader::LoadModel(resource_.modelName, resource_.modelPath, model, animationComp);

        auto m = registry.AddComponent<No::MaterialComponent>(entity);
        m->materials = NoEngine::ModelLoader::GetMaterial("bat");
        // m->materials[0].textureHandle = NoEngine::TextureManager::LoadCovertTexture(enemyResources_.texturePath);
        m->psoName = L"Renderer : Default PSO";
        m->psoId = NoEngine::Render::GetPSOID(m->psoName);
        m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
        m->drawOutline = true;
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
