#include "BossControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include"../../Component/BatBossComponent.h"
#include "../../tag.h"

void BossControlSystem::Update(No::Registry& registry, float deltaTime)
{
   
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        BattBossComponent,
        Boss1Tag>();

    //auto ballView = registry.View<
    //    No::TransformComponent,
    //    No::MaterialComponent,
    //    SphereColliderComponent,
    //    //PhysicsComponent,
    //    //BallStateComponent,
    //    BallTag, DeathFlag>();

    (void)deltaTime;

    for (auto entity : view)
    {
       
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);
        auto* boss = registry.GetComponent<BattBossComponent>(entity);

        if (collider->isCollied)
        {
    
            boss->hp--;
            material->materials[0].color = NoEngine::Color(1.0f, 0.0f, 0.0f, 1.0f);
        } else
        {
            material->materials[0].color = NoEngine::Color(1.0f, 1.0f, 1.0f, 1.0f);
        }


#ifdef USE_IMGUI

        auto* transform = registry.GetComponent<No::TransformComponent>(entity);

            std::string imGuiName = "Boss model" + std::to_string(entity);
            ImGui::Begin(imGuiName.c_str());
            ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
            ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
            ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
            ImGui::Text("hp : %d", boss->hp);
            ImGui::DragFloat3("velocity", &boss->velocity.x, 0.05f);
            ImGui::End();
        
#endif // USE_IMGUI
    }






}

