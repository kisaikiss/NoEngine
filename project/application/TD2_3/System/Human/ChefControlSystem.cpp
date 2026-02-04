#include "ChefControlSystem.h"
#include "../../Component/BallStateComponent.h"
#include "../../Component/PhaseComponent.h"
#include "../../tag.h"

ChefControlSystem::ChefControlSystem()
{
    timer_ = 10.0f;
    isWin_ = false;
    isAmoreStart_ = false;
    isBallHit_ = false;
    idleTimer_ = 0.0f;

    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/chef_amore.mp3", "chef_amore");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/chef_sharenaran.mp3", "chef_sharenaran");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/chef_akan.mp3", "chef_akan");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/chef_ariehen.mp3", "chef_ariehen");

    loseVoice_.clear();
    loseVoice_.push_back("chef_sharenaran");
    loseVoice_.push_back("chef_akan");
    loseVoice_.push_back("chef_ariehen");

}

void ChefControlSystem::Update(No::Registry& registry, float deltaTime)
{

    auto phaseView = registry.View<PhaseComponent>();
    for (auto entity : phaseView) {
        auto* phase = registry.GetComponent<PhaseComponent>(entity);
        if (phase->phase != Phase::TWO)
            return;
    }

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        No::AnimatorComponent,
        ChefTag>();

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


    auto humanParentView = registry.View<No::TransformComponent, EnemyHumanTag>();
    No::TransformComponent* parent = nullptr;

    for (auto entity : humanParentView) {
        parent = registry.GetComponent<No::TransformComponent>(entity);
    }

    for (auto entity : view)
    {
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        transform->parent = parent;

        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* animation = registry.GetComponent<No::AnimatorComponent>(entity);

        if (isOut) {
            if (!isAmoreStart_) {
                animation->currentAnimation = 1;
                //material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/faceJoy.png");
                No::SoundEffectPlay("chef_amore", 1.0f);
                isAmoreStart_ = true;
            }

        } else if (isEnemyDead) {

            if (!isAmoreStart_) {
                //アモーレ開始していないとき
                if (!isBallHit_) {
                    //エネミーが死んだとき
                    animation->currentAnimation = 3;

                    int randVoice = rand() % loseVoice_.size();
                    No::SoundEffectPlay(loseVoice_[randVoice], 1.0f);
                    isBallHit_ = true;
                }
            }
        } else {

            idleTimer_ += deltaTime;

            if (animation->time + deltaTime >= animation->animation[animation->currentAnimation].duration) {

                isAmoreStart_ = false;
                isBallHit_ = false;

                if (timer_ >= 20.0f) {
                    animation->currentAnimation = 2;
                    timer_ = 0.0f;
                } else {
                    animation->currentAnimation = 0;
                }
            }


            if (!isAmoreStart_) {

                timer_ += deltaTime;

                //if (animation->currentAnimation == 3 || timer_ >= 1.5f && timer_ <= 2.0f) {
                //    material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face2.png");
                //} else {
                //    material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/batGirl/face.png");
                //}

            }

        }


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
        (void)material;
#endif // USE_IMGUI

    }
}
