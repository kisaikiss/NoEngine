#include "BatGirlControlSystem.h"

#include "../../tag.h"

BatGirlControlSystem::BatGirlControlSystem()
{
    timer_ = 0.0f;
    idleRandNum_ = 0;
}

void BatGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        No::AnimatorComponent,
        BatGirlTag>();

   
    timer_ += deltaTime;
    timer_ = fmodf(timer_, 3.0f);

    for (auto entity : view)
    {
        auto*  animation = registry.GetComponent<No::AnimatorComponent>(entity);


        if (animation->time + deltaTime >= animation->animation[animation->currentAnimation].duration ) {
         
            do {
                idleRandNum_ = rand() % 4;
             
            } while (animation->currentAnimation == idleRandNum_);
            animation->currentAnimation = idleRandNum_;
        }

        auto* material = registry.GetComponent<No::MaterialComponent>(entity);

        if (timer_ <= 1.5f || timer_ >= 2.0f) {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face.png");
        } else {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face2.png");
        }
#ifdef USE_IMGUI
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
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
