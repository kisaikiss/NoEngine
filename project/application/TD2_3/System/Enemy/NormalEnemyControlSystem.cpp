#include "NormalEnemyControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../tag.h"

void NormalEnemyControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        NormalEnemyTag>();

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

        transform->translate = velocity_ * deltaTime;

        if (collider->isCollied)
        {
            material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
        } else
        {
            material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}
