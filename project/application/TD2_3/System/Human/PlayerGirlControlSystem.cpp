#include "PlayerGirlControlSystem.h"
#include "../../tag.h"

PlayerGirlControlSystem::PlayerGirlControlSystem()
{
    timer_ = 0.0f;
}

void PlayerGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        PlayerGirlTag>();

    timer_ += deltaTime;
    timer_ = fmodf(timer_, 3.0f);

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);

#ifdef USE_IMGUI

        if (timer_ <= 2.5f ) {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face.png");
        } else {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face2.png");
        }

        std::string imGuiName = "playerGirlTag";
        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::DragFloat4("faceColor1", &material->materials[1].color.r, 0.04f);
        ImGui::End();

#endif // USE_IMGUI
    }

}
