#include "BossControlSystem.h"
#include "../../Component/ColliderComponent.h"
#include "../../tag.h"

void BossControlSystem::Update(No::Registry& registry, float deltaTime)
{
   
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        SphereColliderComponent,
        Boss1Tag>();

    (void)deltaTime;

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* collider = registry.GetComponent<SphereColliderComponent>(entity);


        if (transform->translate.x <= -5.0f || transform->translate.x >= 5.0f)speed_ *= -1.0f;

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
    
            ImGui::End();
        
#endif // USE_IMGUI
    }






}

