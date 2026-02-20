#include "ResultPlayerGirlControlSystem.h"
#include "../../Component/PlayerStatusComponent.h"
#include "../../tag.h"

void ResultPlayerGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{
    //ゲーム終了時
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        No::AnimatorComponent,
        PlayerGirlTag>();

    for (auto entity : view)
    { 

        auto* animation = registry.GetComponent<No::AnimatorComponent>(entity);

        if (PlayerStatusComponent::isGameOver) {

            if (!isGameOverAnimationStart_) {
                animation->currentAnimation = 11;
                isGameOverAnimationStart_ = true;
            }

            if (isGameOverAnimationStart_) {
                if (animation->time + deltaTime >= animation->animation[animation->currentAnimation].duration) {
                    animation->currentAnimation = 10;
                }
            }
       

        } else if (PlayerStatusComponent::isGameClear) {

            animation->currentAnimation = 9;
        }
#ifdef USE_IMGUI

        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        std::string imGuiName = "playerGirlTag";
        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::DragFloat("animD", &animation->animation[animation->currentAnimation].duration, 0.04f);
        ImGui::DragFloat("animTime", &animation->time, 0.04f);
        ImGui::Text("currentAnim %d", animation->currentAnimation);
        ImGui::End();

#endif // USE_IMGUI
    }

}
