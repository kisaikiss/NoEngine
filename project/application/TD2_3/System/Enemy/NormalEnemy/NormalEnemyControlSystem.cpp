#include "NormalEnemyControlSystem.h"
#include "../../../Component/ColliderComponent.h"
#include "../../../Component/VausStateComponent.h"
//#include"../../Component/NormalEnemyComponent.h"
#include "../../../Component/BallStateComponent.h"
#include"engine/Math/Easing.h"

#include "../../../tag.h"
#include "engine/Functions/Renderer/Primitive.h"
#include"../EnemyCommonMove.h"


using namespace No;
using namespace NoEngine;

NormalEnemyControlSystem::NormalEnemyControlSystem()
{

}

void NormalEnemyControlSystem::Update(No::Registry& registry, float deltaTime)
{

    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        NormalEnemyComponent,
        SphereColliderComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    for (auto entity : view)
    {

        /*       auto* material = registry.GetComponent<No::MaterialComponent>(entity);*/
     
        auto* enemy = registry.GetComponent<NormalEnemyComponent>(entity);

        // 初回だけ stateManager を作る 
        if (!enemy->isStarted_) {
            enemy->stateManager = std::make_shared<EnemyStateManager<NormalEnemyComponent>>();
            enemy->stateManager->Start(enemy);
            enemy->stateManager->ChangeState<EnemyAppear<NormalEnemyComponent>>(registry);
            enemy->isStarted_ = true;
        }

        enemy->stateManager->Update(registry, deltaTime);

#ifdef USE_IMGUI
        
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

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
        ImGui::Text("hp %d", enemy->hp);
        ImGui::End();

#endif // USE_IMGUI

    }


}

void EnemyAppear<NormalEnemyComponent>::Enter(No::Registry registry, No::Entity entity)
{

    (void)registry;
    timer_ = 0.0f;
    auto* transform = registry.GetComponent<TransformComponent>(entity);
    transform->scale = Vector3::ZERO;
}

void EnemyAppear<NormalEnemyComponent>::Update(No::Registry registry,No::Entity entity,float deltaTime) {


    TimerUpdate(timer_, deltaTime);

    auto* transform = registry.GetComponent<TransformComponent>(entity);

    if (timer_ <= 7.5f) {
        float timer = timer_ / 7.5f;
        transform->scale = NoEngine::Easing::EaseInOutBack(Vector3::ZERO, Vector3::UNIT_SCALE, timer);
    } else {
        stateManager_->ChangeState<EnemyMove>(registry);
    }

}

void EnemyAppear<NormalEnemyComponent>::Exit(No::Registry registry, No::Entity entity)
{
    auto* transform = registry.GetComponent<TransformComponent>(entity);
    transform->scale = Vector3::UNIT_SCALE;

}

void EnemyMove::Enter(No::Registry registry, No::Entity entity)
{
    (void)registry;
    (void)entity;
    theta_ = rand() % 314 * 0.01f;
}


void EnemyMove::Update(No::Registry registry,No::Entity entity,float deltaTime)
{
    auto* material = registry.GetComponent<MaterialComponent>(entity);

    auto* transform = registry.GetComponent<No::TransformComponent>(entity);
    theta_ += deltaTime * PI;
    transform->translate.y = ownerType_->defaultTranslate_.y + sinf(theta_) * 0.5f;

    auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

    if (collider->isCollied)
    {
        ownerType_->stateManager->ChangeState<EnemyHit<NormalEnemyComponent>>(registry);
    } else
    {
        material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
    }

}

void EnemyMove::Exit(No::Registry registry, No::Entity entity)
{
    (void)registry;

}

void EnemyHit<NormalEnemyComponent>::Enter(No::Registry registry, No::Entity entity)
{

    timer_ = 0.0f;

    auto* material = registry.GetComponent<MaterialComponent>(entity);
    material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
    auto* enemy = registry.GetComponent<NormalEnemyComponent>(entity);
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

void EnemyHit<NormalEnemyComponent>::Update(No::Registry registry,No::Entity entity,float deltaTime)
{

    TimerUpdate(timer_, deltaTime);

    auto* transform = registry.GetComponent<No::TransformComponent>(entity);

    PoyoPoyo(*transform, timer_, 10.0f, 0.25f, Vector3::UNIT_SCALE);

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
            stateManager_->ChangeState<EnemyDie<NormalEnemyComponent>>(registry);
        } else {
            stateManager_->ChangeState<EnemyMove>(registry);
        }
    }

}

void EnemyHit<NormalEnemyComponent>::Exit(No::Registry registry, No::Entity entity)
{

    auto* material = registry.GetComponent<MaterialComponent>(entity);
    material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
    auto* transform = registry.GetComponent<No::TransformComponent>(entity);
    transform->scale = Vector3::UNIT_SCALE;

    ballPhysics_ = nullptr;

}

void EnemyDie<NormalEnemyComponent>::Enter(No::Registry registry, No::Entity entity)
{
    (void)registry;
    timer_ = 0.0f;
}

void EnemyDie<NormalEnemyComponent>::Update(No::Registry registry,No::Entity entity,float deltaTime)
{
    TimerUpdate(timer_, deltaTime);

    auto* transform = registry.GetComponent<TransformComponent>(entity);

    if (timer_ <= 3.0f) {
        float timer = timer_ / 3.0f;
        transform->scale = NoEngine::Easing::EaseInOutBack(Vector3::UNIT_SCALE, Vector3::ZERO, timer);
    } else {
        auto* deathFlag = registry.GetComponent<DeathFlag>(entity);
        deathFlag->isDead = true;
    }

}

void EnemyDie<NormalEnemyComponent>::Exit(No::Registry registry, No::Entity entity)
{
    auto* deathFlag = registry.GetComponent<DeathFlag>(entity);
    deathFlag->isDead = true;

}
