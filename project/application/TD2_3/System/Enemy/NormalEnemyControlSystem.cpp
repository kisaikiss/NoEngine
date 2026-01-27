#include "NormalEnemyControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../Component/VausStateComponent.h"
#include"../../Component/NormalEnemyComponent.h"
#include "../../tag.h"
#include "engine/Functions/Renderer/Primitive.h"

using namespace No;
using namespace NoEngine;

NormalEnemyControlSystem::NormalEnemyControlSystem()
{
    enemyStateManager_.Start(this);
    timer_ = 0.0f;
}

void NormalEnemyControlSystem::Update(No::Registry& registry, float deltaTime)
{
    //auto view = registry.View<
    //    NormalEnemyTag,
    //    No::MaterialComponent,
    //    SphereColliderComponent,
    //    NormalEnemyComponent,
    //    No::TransformComponent,
    //    DeathFlag>();

    if (timer_ >= 5.0f) {
        enemyStateManager_.ChangeState<EnemyChase>(registry);
    } else {
        if (timer_ == 0.0f) { enemyStateManager_.ChangeState<EnemyAppear>(registry); }
        timer_ += deltaTime;
    }

    //enemyStateManager_.Update(registry);

    //for (auto entity : view)
    //{
    //    auto* material = registry.GetComponent<No::MaterialComponent>(entity);
    //    auto* collider = registry.GetComponent<SphereColliderComponent>(entity);

    //    if (collider->isCollied) {

    //        material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
    //        /*          enemy->velocity *= -10.0f;
    //                  enemy->hp--;*/

    //                  /*       if (enemy->hp <= 0) {


    //                         }*/
    //    } else {
    //        material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
    //    }

    //}
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        NormalEnemyComponent>();

    No::TransformComponent* vausTransform = nullptr;

    auto vausView = registry.View<
        No::TransformComponent,
        VausTag>();

    for (auto vausEntity : vausView) {
        vausTransform = registry.GetComponent<TransformComponent>(vausEntity);
    }

    if (vausTransform == nullptr) {
        return;
    }
    (void)deltaTime;

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);
        auto* enemy = registry.GetComponent<NormalEnemyComponent>(entity);


        Vector3  direction = vausTransform->translate - transform->translate;
        direction.z = 0.0f;
        float length = direction.Length();
        if (length != 0) {
            direction /= length;
        }
        float speed = 2.0f;

        enemy->velocity.x = speed * direction.x;
        enemy->velocity.y = speed * direction.y;

        transform->translate += enemy->velocity * deltaTime;
        Primitive::DrawSphere(transform->translate, collider->radius, NoEngine::Color(1.0f, 0.f, 0.f));


        if (collider->isCollied)
        {
            material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
        } else
        {
            material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        }


#ifdef USE_IMGUI

        std::string imGuiName = "model" + std::to_string(entity);
        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::Text("collied %s", collider->isCollied ? "true" : "false");
		ImGui::Text("colliedWith %u", static_cast<uint32_t>(collider->colliedWith));
        ImGui::Text("colliedEntity %u", static_cast<uint32_t>(collider->colliedEntity));
        ImGui::End();

#endif // USE_IMGUI
    }

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

void EnemyAppear::Update(No::Registry& registry, NormalEnemyControlSystem* ownerType, float deltaTime)
{
    (void)ownerType;
	(void)deltaTime;
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

void EnemyChase::Update(No::Registry& registry, NormalEnemyControlSystem* ownerType,float deltaTime)
{
    (void)ownerType;

    No::TransformComponent* vausTransform = nullptr;

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        NormalEnemyComponent,
        NormalEnemyTag>();

    auto vausView = registry.View<
        No::TransformComponent,
        VausTag>();

    for (auto vausEntity : vausView) {
        vausTransform = registry.GetComponent<TransformComponent>(vausEntity);
    }

    if (vausTransform == nullptr) {
        return;
    }


    for (auto entity : view)
    {
        //auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* enemy = registry.GetComponent<NormalEnemyComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);
        Vector3  direction = vausTransform->translate - transform->translate;
        direction.z = 0.0f;
        float length = direction.Length();
        if (length != 0) {
            direction /= length;
        }
        float speed = 2.0f;

        enemy->velocity.x = speed * direction.x;
        enemy->velocity.y = speed * direction.y;

        transform->translate += enemy->velocity * deltaTime;
        Primitive::DrawSphere(transform->translate, collider->radius, NoEngine::Color(1.0f, 0.f, 0.f));

    }


}

void EnemyChase::Exit(No::Registry& registry, NormalEnemyControlSystem* ownerType)
{
    (void)registry;
    (void)ownerType;
}
