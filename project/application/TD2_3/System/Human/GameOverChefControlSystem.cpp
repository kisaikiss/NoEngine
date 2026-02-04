#include "GameOverChefControlSystem.h"
#include "../../tag.h"
#include "../../Component/PhaseComponent.h"

void GameOverChefControlSystem::Update(No::Registry& registry, float deltaTime)
{


    if (PhaseComponent::phase != Phase::TWO) {
        return;
    }

    (void)deltaTime;

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        No::AnimatorComponent,
        ChefTag>();

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* animation = registry.GetComponent<No::AnimatorComponent>(entity);

        //ゲーム終了時
        animation->currentAnimation = 4;

#ifdef USE_IMGUI

        ImGui::Begin("Chef");
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::DragFloat4("faceColor1", &material->materials[1].color.r, 0.04f);
        ImGui::DragFloat("animD", &animation->animation[animation->currentAnimation].duration, 0.04f);
        ImGui::DragFloat("animTime", &animation->time, 0.04f);
        ImGui::Text("currentAnim %d", animation->currentAnimation);
        ImGui::End();
#else
        (void)transform;
        (void)material;
#endif // USE_IMGUI

    }
}
