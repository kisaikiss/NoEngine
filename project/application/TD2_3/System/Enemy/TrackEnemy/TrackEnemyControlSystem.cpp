#include "TrackEnemyControlSystem.h"
#include "../../../Component/ColliderComponent.h"
#include "../../../Component/VausStateComponent.h"
#include"../../../Component/TrackEnemyComponent.h"
#include "../../../Component/BallStateComponent.h"
#include"engine/Math/Easing.h"

#include "../../../tag.h"
#include "engine/Functions/Renderer/Primitive.h"
#include"../../Enemy/EnemyCommonMove.h"
#include"../NormalEnemy/NormalEnemyControlSystem.h"

using namespace No;
using namespace NoEngine;


void TrackEnemyControlSystem::Update(No::Registry& registry, float deltaTime)
{

    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        TrackEnemyComponent,
        SphereColliderComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    for (auto entity : view)
    {

        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);
        auto* enemy = registry.GetComponent<TrackEnemyComponent>(entity);

        // 初回だけ stateManager を作る 
        if (!enemy->isStarted_) {
            enemy->stateManager = std::make_shared<EnemyStateManager<TrackEnemyComponent>>();
            enemy->stateManager->Start(enemy);
            enemy->stateManager->ChangeState<EnemyAppear<TrackEnemyComponent>>(registry);
            enemy->isStarted_ = true;
        }


        if (collider->isCollied)
        {
            enemy->stateManager->ChangeState<EnemyHit<TrackEnemyComponent>>(registry);
        } else
        {
            material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        }

        enemy->stateManager->Update(registry, deltaTime);

#ifdef USE_IMGUI
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

        std::string imGuiName = "model" + std::to_string(entity);
        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::Text("collied %s", collider->isCollied ? "true" : "false");
        ImGui::Text("colliedWith %u", static_cast<uint32_t>(collider->colliedWith));
        ImGui::Text("colliedEntity %u", static_cast<uint32_t>(collider->colliedEntity));

        ImGui::Text("isDead %s", deathFlag->isDead ? "true" : "false");

        ImGui::End();

#endif // USE_IMGUI

    }


}


void EnemyAppear<TrackEnemyComponent>::Enter(No::Registry& registry)
{
    (void)registry;
    timer_ = 0.0f;
}

void EnemyAppear<TrackEnemyComponent>::Update(No::Registry& registry, float deltaTime) {


    TimerUpdate(timer_, deltaTime);

    auto* transform = registry.GetComponent<TransformComponent>(ownerType_->entity);

    if (timer_ <= 3.0f) {
        float timer = timer_ / 3.0f;
        transform->scale = NoEngine::Easing::EaseInOutBack(Vector3::ZERO, Vector3::UNIT_SCALE, timer);
    }

    if (timer_ >= 5.0f) {
        stateManager_->ChangeState<EnemyChase>(registry);
    }
}

void EnemyAppear<TrackEnemyComponent>::Exit(No::Registry& registry)
{
    auto* transform = registry.GetComponent<TransformComponent>(ownerType_->entity);
    transform->scale = Vector3::UNIT_SCALE;

}

void EnemyChase::Enter(No::Registry& registry)
{
    (void)registry;

}


void EnemyChase::Update(No::Registry& registry, float deltaTime)
{

    No::TransformComponent* targetTransform = nullptr;

    auto ballView = registry.View<
        BallTag,
        No::TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    for (auto ballEntity : ballView) {
        targetTransform = registry.GetComponent<TransformComponent>(ballEntity);
    }

    if (targetTransform == nullptr) {
        return;
    }

    auto* transform = registry.GetComponent<No::TransformComponent>(ownerType_->entity);
    Vector3 direction = GatTargetDir(transform->translate, targetTransform->translate);

    float speed = 1.0f;
    ownerType_->velocity.x = speed * direction.x;
    ownerType_->velocity.y = speed * direction.y;
    transform->translate += ownerType_->velocity * deltaTime;

    LookTarget(*transform, targetTransform->translate);



}

void EnemyChase::Exit(No::Registry& registry)
{
    (void)registry;

}

void EnemyHit<TrackEnemyComponent>::Enter(No::Registry& registry)
{

    timer_ = 0.0f;

    auto* material = registry.GetComponent<MaterialComponent>(ownerType_->entity);
    material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
    auto* enemy = registry.GetComponent<TrackEnemyComponent>(ownerType_->entity);
    //HPを減らす
    enemy->hp--;

    auto ballView = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        PhysicsComponent,
        BallStateComponent,
        BallTag, DeathFlag>();

    for (auto entity : ballView)
    {
        ballPhysics_ = registry.GetComponent<PhysicsComponent>(entity);
    }
    No::SoundEffectPlay("batDie", 0.5f);
}

void EnemyHit<TrackEnemyComponent>::Update(No::Registry& registry, float deltaTime)
{

    TimerUpdate(timer_, deltaTime);

    auto ballView = registry.View<
        BallTag,
        No::TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    No::TransformComponent* targetTransform = nullptr;

    for (auto ballEntity : ballView) {
        targetTransform = registry.GetComponent<TransformComponent>(ballEntity);
    }

    auto* transform = registry.GetComponent<No::TransformComponent>(ownerType_->entity);

    PoyoPoyo(*transform, timer_, 10.0f, 0.25f, Vector3::UNIT_SCALE);

    if (targetTransform != nullptr) {
        LookTarget(*transform, targetTransform->translate);
    }

    Vector3 vel = { 0.0f,0.0f,0.0f };

    if (timer_ <= 0.5f) {

        if (ballPhysics_ == nullptr) {
            vel = ownerType_->velocity;
            vel.z = 0.0f;
            transform->translate += vel * -1.0f * deltaTime;
        } else {
            vel = ballPhysics_->velocity;
            vel.z = 0.0f;
            transform->translate += vel * -0.5f * deltaTime;
        }

    }

    if (timer_ >= 1.0f) {

        if (ownerType_->hp <= 0) {
            stateManager_->ChangeState<EnemyDie<TrackEnemyComponent>>(registry);
        } else {
            stateManager_->ChangeState<EnemyChase>(registry);
        }
    }


}

void EnemyHit<TrackEnemyComponent>::Exit(No::Registry& registry)
{

    auto* material = registry.GetComponent<MaterialComponent>(ownerType_->entity);
    material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
    auto* transform = registry.GetComponent<No::TransformComponent>(ownerType_->entity);
    transform->scale = Vector3::UNIT_SCALE;

    ballPhysics_ = nullptr;

}

void EnemyDie<TrackEnemyComponent>::Enter(No::Registry& registry)
{
    (void)registry;

}

void EnemyDie<TrackEnemyComponent>::Update(No::Registry& registry, float deltaTime)
{
    TimerUpdate(timer_, deltaTime);

    auto* transform = registry.GetComponent<TransformComponent>(ownerType_->entity);

    if (timer_ <= 3.0f) {
        float timer = timer_ / 3.0f;
        transform->scale = NoEngine::Easing::EaseInOutBack(Vector3::UNIT_SCALE, Vector3::ZERO, timer);
    } else {
        auto* deathFlag = registry.GetComponent<DeathFlag>(ownerType_->entity);

        if (deathFlag->isDead) {
            return;
        }

        deathFlag->isDead = true;

    }

}

void EnemyDie<TrackEnemyComponent>::Exit(No::Registry& registry)
{
    (void)registry;

}
