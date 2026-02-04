#include "GameOverBatControlSystem.h"
#include "../../tag.h"
#include "../../Component/PhaseComponent.h"

void GameOverBatControlSystem::Update(No::Registry& registry, float deltaTime)
{
    (void)deltaTime;
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        No::AnimatorComponent,
        BatGirlTag>();

    if (PhaseComponent::phase != Phase::ONE) {
        return;
    }

    for (auto entity : view)
    {
        auto* animation = registry.GetComponent<No::AnimatorComponent>(entity);
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        //ゲーム終了時
        animation->currentAnimation = 6;

        NoEngine::Quaternion rotate1;
        rotate1.FromAxisAngle(NoEngine::Vector3::UP, -PI*0.375f);
        NoEngine::Quaternion rotate2;
        rotate2.FromAxisAngle(NoEngine::Vector3::RIGHT, PI);

        transform->rotation = rotate2 * rotate1;

#ifdef USE_IMGUI
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
    
        std::string imGuiName = "batGirl";
        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::DragFloat4("faceColor1", &material->materials[1].color.r, 0.04f);
        ImGui::DragFloat("animD", &animation->animation[animation->currentAnimation].duration, 0.04f);
        ImGui::DragFloat("animTime", &animation->time, 0.04f);
        ImGui::Text("currentAnim %d", animation->currentAnimation);
        ImGui::End();

#endif // USE_IMGUI

    }
}
