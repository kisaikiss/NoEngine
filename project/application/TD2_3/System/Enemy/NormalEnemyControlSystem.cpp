#include "NormalEnemyControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../tag.h"

using namespace No;
using namespace NoEngine;

NormalEnemyControlSystem::NormalEnemyControlSystem(No::Registry& registry)
{
    enemyStateManager_.Start(this);
    enemyStateManager_.ChangeState<EnemyChase>(registry);
    timer_ = 0.0f;
}

void NormalEnemyControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        NormalEnemyTag>();

    timer_ += deltaTime;

    if (timer_ >= 5.0f) {
        enemyStateManager_.ChangeState<EnemyChase>(registry);
    }

    for (auto entity : view)
    {
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

        if (collider->isCollied) {
            material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
        } else{
             material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    enemyStateManager_.Update(registry);
}

void EnemyAppear::Enter(No::Registry& registry, NormalEnemyControlSystem* ownerType)
{
    (void)ownerType;

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        NormalEnemyTag>();

    //for (auto entity : view)
    //{
    ///*    auto* transform = registry.GetComponent<No::TransformComponent>(entity);*/
   

    //}


}

void EnemyAppear::Update(No::Registry& registry, NormalEnemyControlSystem* ownerType)
{
    (void)ownerType;

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        NormalEnemyTag>();

    //for (auto entity : view)
    //{
    //    /*    auto* transform = registry.GetComponent<No::TransformComponent>(entity);*/
    //    auto* material = registry.GetComponent<No::MaterialComponent>(entity);



    //}

}

void EnemyAppear::Exit(No::Registry& registry, NormalEnemyControlSystem* ownerType)
{
    (void)registry;
    (void)ownerType;
}

void EnemyChase::Enter(No::Registry& registry, NormalEnemyControlSystem* ownerType)
{
    (void)registry;
    (void)ownerType;

}

void EnemyChase::Update(No::Registry& registry, NormalEnemyControlSystem* ownerType)
{
    (void)ownerType;
 
    No::TransformComponent* playerTransform = nullptr;

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        NormalEnemyTag>();

    auto ballView = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        BallTag>();

    for (auto playerEntity : ballView) {
        playerTransform = registry.GetComponent<  No::TransformComponent>(playerEntity);
    }

    if (playerTransform == nullptr) {
        return;
    }


    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
       
        Vector3 direction = playerTransform->translate - transform->translate;
        float length = direction.Length();
        if (length != 0) {
            direction /= length;
        }

        transform->translate += direction*0.016f;
  
    }

}

void EnemyChase::Exit(No::Registry& registry, NormalEnemyControlSystem* ownerType)
{
    (void)registry;
    (void)ownerType;
}
