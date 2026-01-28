#include "NormalEnemyControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../Component/VausStateComponent.h"
//#include"../../Component/NormalEnemyComponent.h"
#include "../../Component/BallStateComponent.h"
#include"engine/Math/Easing.h"

#include "../../tag.h"
#include "engine/Functions/Renderer/Primitive.h"

using namespace No;
using namespace NoEngine;

NormalEnemyControlSystem::NormalEnemyControlSystem()
{

    isApper_ = false;
    stateManager_ = std::make_unique <EnemyStateManager<NormalEnemyComponent>>();
   
}

void NormalEnemyControlSystem::Update(No::Registry& registry, float deltaTime)
{

    (void)deltaTime;



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
        if (!isApper_) { 
            
            auto* enemy = registry.GetComponent<NormalEnemyComponent>(entity);
            
            stateManager_->Start(enemy);
            stateManager_->ChangeState<EnemyAppear>(registry); isApper_ = true; }


        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

        auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

        if (collider->isCollied)
        {
            stateManager_->ChangeState<EnemyHit>(registry);
        } else
        {
            material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        }
    
        stateManager_->Update(registry, deltaTime);

#ifdef USE_IMGUI

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


void EnemyAppear::Enter(No::Registry& registry)
{


    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        SphereColliderComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    timer_ = 0.0f;
}




void EnemyAppear::Update(No::Registry& registry,  float deltaTime){


    TimerUpdate(timer_, deltaTime);

    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        SphereColliderComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();


    for (auto entity : view) {
        auto* transform = registry.GetComponent<TransformComponent>(entity);

        if (timer_ <= 3.0f) {
            float timer = timer_ / 3.0f;
            transform->scale = EaseInOutBack(Vector3::ZERO, Vector3::UNIT_SCALE, timer);
        }


        if (timer_ >= 5.0f) {
            stateManager_->ChangeState<EnemyChase>(registry);

        }
    }


}

void EnemyAppear::Exit(No::Registry& registry)
{
   
    auto view = registry.View<
        No::MaterialComponent,
        SphereColliderComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    for (auto entity : view) {
        auto* transform = registry.GetComponent<TransformComponent>(entity);
        transform->scale = Vector3::UNIT_SCALE;
    }

}

void EnemyChase::Enter(No::Registry& registry)
{
    (void)registry;

}


void EnemyChase::Update(No::Registry& registry,float deltaTime)
{

    No::TransformComponent* targetTransform = nullptr;

    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        SphereColliderComponent,
        NormalEnemyComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

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


    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        Vector3 direction = GatTargetDir(transform->translate, targetTransform->translate);

        float speed = 2.0f;
        ownerType_->velocity.x = speed * direction.x;
        ownerType_->velocity.y = speed * direction.y;
        transform->translate += ownerType_->velocity * deltaTime;

        LookTarget(*transform, targetTransform->translate);

    }

}

void EnemyChase::Exit(No::Registry& registry)
{
    (void)registry;

}

void EnemyHit::Enter(No::Registry& registry)
{

    timer_ = 0.0f;

    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        SphereColliderComponent,
        NormalEnemyComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    for (auto entity : view)
    {
        auto* material = registry.GetComponent<MaterialComponent>(entity);
        material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);        
        auto* enemy = registry.GetComponent<NormalEnemyComponent>(entity);
        //HPを減らす
        enemy->hp--;
    }

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

}

void EnemyHit::Update(No::Registry& registry, float deltaTime)
{

    TimerUpdate(timer_, deltaTime);

    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        SphereColliderComponent,
        NormalEnemyComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    auto ballView = registry.View<
        BallTag,
        No::TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    No::TransformComponent* targetTransform = nullptr;

    for (auto ballEntity : ballView) {
        targetTransform = registry.GetComponent<TransformComponent>(ballEntity);
    }

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);

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
                transform->translate += vel * -0.25f * deltaTime;
            }

        }

        if (timer_ >= 1.0f) {

            if (ownerType_->hp <= 0) {
               stateManager_->ChangeState<EnemyDie>(registry);
            } else {
                stateManager_->ChangeState<EnemyChase>(registry);
            }
        }

    }




}

void EnemyHit::Exit(No::Registry& registry)
{


    auto view = registry.View <
        NormalEnemyTag,

        SphereColliderComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    for (auto entity : view)
    {
        auto* material = registry.GetComponent<MaterialComponent>(entity);
        material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        transform->scale = Vector3::UNIT_SCALE;

    }

    ballPhysics_ = nullptr;

}

void PoyoPoyo(No::TransformComponent& transform, float timer, float speed, float scaling, const NoEngine::Vector3& defaultScale)
{
    float theta = std::numbers::pi_v<float>*speed * timer;
    transform.scale.x = defaultScale.x + cos(theta) * scaling;
    transform.scale.y = defaultScale.y + sin(theta) * scaling;
}

void TimerUpdate(float& timer, float& deltaTime)
{
    timer += deltaTime;
}

void LookTarget(No::TransformComponent& transform, const Vector3& target)
{
    Vector3 direction = target - transform.translate;
    direction.z = 0.0f;
    // 正規化して方向ベクトルにする
    direction = direction.Normalize();

    float angle = std::atan2(direction.y, direction.x);

    transform.rotation.FromAxisAngle(Vector3::UP, PI + angle);

}


Vector3 GatTargetDir(Vector3& translate, const Vector3& target)
{
    Vector3 direction = target - translate;

    direction.z = 0.0f;

    float length = direction.Length();

    if (length != 0) {
        direction /= length;
    } else {
        direction = { Vector3::ZERO };
    }

    return direction;
}

float EaseInOutBackT(const float& x) {

    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;

    return x < 0.5f
        ? (std::powf(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
        : (std::powf(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

Vector3 EaseInOutBack(const Vector3& start, const Vector3& end, float t) {
    float time = EaseInOutBackT(t);
    return NoEngine::Easing::Lerp(start, end, time);
}

void EnemyDie::Enter(No::Registry& registry)
{
    (void)registry;

}

void EnemyDie::Update(No::Registry& registry,float deltaTime)
{
    TimerUpdate(timer_, deltaTime);

    auto view = registry.View <
        NormalEnemyTag,
        DeathFlag,
        SphereColliderComponent,
        TransformComponent,
        No::MeshComponent,
        No::MaterialComponent>();

    for (auto entity : view) {
        auto* transform = registry.GetComponent<TransformComponent>(entity);

        if (timer_ <= 3.0f) {
            float timer = timer_ / 3.0f;
            transform->scale = EaseInOutBack(Vector3::UNIT_SCALE, Vector3::ZERO, timer);
        } else {
            //auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

            //if (deathFlag->isDead) {
            //    return;
            //}

            //deathFlag->isDead = true;

        }

    }


}

void EnemyDie::Exit(No::Registry& registry)
{
    (void)registry;
  
}
