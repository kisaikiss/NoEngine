#include "BatGirlControlSystem.h"

#include "../../tag.h"

BatGirlControlSystem::BatGirlControlSystem()
{
    timer_ = 0.0f;
}

void BatGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,

        BatGirlTag>();


    (void)deltaTime;


#ifdef USE_IMGUI
    for (auto entity : view)
    {

        auto* transform = registry.GetComponent<No::TransformComponent>(entity);

        auto* material = registry.GetComponent<No::MaterialComponent>(entity);

        timer_ += deltaTime;
        timer_ = fmodf(timer_, 3.0f);

        if (timer_ <= 1.5f|| timer_ >= 2.0f) {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face.png");
        } else {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face2.png");
        }
        std::string imGuiName = "batGirl";
        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat3("translate", &transform->translate.x, 0.05f);
        ImGui::DragFloat3("scale", &transform->scale.x, 0.05f);
        ImGui::DragFloat4("rotate", &transform->rotation.x, 0.04f);
        ImGui::DragFloat4("faceColor1", &material->materials[1].color.r, 0.04f);
        ImGui::End();
    }
#endif // USE_IMGUI
  


}
