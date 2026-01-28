#include "BatGirlControlSystem.h"

#include "../../tag.h"

void BatGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,

        BatGirlTag>();


    (void)deltaTime;

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);


#ifdef USE_IMGUI

        std::string imGuiName = "batGirl";
        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::DragFloat4("faceColor1", &material->materials[1].color.r, 0.04f);
        ImGui::End();

#endif // USE_IMGUI
    }


}
