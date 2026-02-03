#include "BatGirlControlSystem.h"
#include "../../Component/BallStateComponent.h"
#include "../../tag.h"

BatGirlControlSystem::BatGirlControlSystem()
{
    timer_ = 0.0f;
    idleRandNum_ = 0;
    isWin_ = false;
    isLaughStart_ = false;
    idleTimer_ = 0.0f;
}

void BatGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        No::AnimatorComponent,
        BatGirlTag>();

    auto ballView = registry.View<
        BallStateComponent,
        BallTag, DeathFlag>();

   


    bool isOut = false;

    for (auto ballEntity : ballView)
    {
        auto* ball = registry.GetComponent<BallStateComponent>(ballEntity);
        //ballの状態を取得する
            isOut = ball->isOut;
    }

    bool isEnemyDead = false;

    auto normalEnemyView = registry.View <
        BatTag,
        DeathFlag>();

    for (auto normalEnemyEntity : normalEnemyView)
    {
        auto* deathFlag = registry.GetComponent<DeathFlag>(normalEnemyEntity);
        if (deathFlag->isDead) {
            //もし敵に当たったら
            isEnemyDead = true;
            break;

        }
    }

    for (auto entity : view)
    {

        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto*  animation = registry.GetComponent<No::AnimatorComponent>(entity);

        if (isOut) {
            if (!isLaughStart_) {
                animation->currentAnimation = 6;
                material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/faceJoy.png");
                No::SoundEffectPlay("batGirlLaugh", 0.5f);
                isLaughStart_ = true;
            }
  
        } else if(isEnemyDead){
            //エネミーが死んだとき
            animation->currentAnimation = 5;

        } else {

            idleTimer_ += deltaTime;

            if (animation->time + deltaTime >= animation->animation[animation->currentAnimation].duration) {

                isLaughStart_ = false;

                if (timer_ >= 20.0f) {

                    do {
                        idleRandNum_ = rand() % 4 + 1;
                    } while (animation->currentAnimation == idleRandNum_);
                    animation->currentAnimation = idleRandNum_;
                    timer_ = 0.0f;

                } else {
                    animation->currentAnimation = 0;
                }
            }

  
            if (!isLaughStart_) {

                timer_ += deltaTime;

                if (animation->currentAnimation == 3 || timer_ >= 1.5f && timer_ <= 2.0f) {
                    material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face2.png");
                } else {
                    material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face.png");
                }

            }
        
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
