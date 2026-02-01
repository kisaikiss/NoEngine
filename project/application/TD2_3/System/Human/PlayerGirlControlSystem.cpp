#include "PlayerGirlControlSystem.h"
#include "../../Component/BallStateComponent.h"
#include "../../tag.h"

PlayerGirlControlSystem::PlayerGirlControlSystem()
{    //strings_.push_back("voice_checkmate");
    timer_ = 0.0f;
    voiceTimer_ = 0.0f;

    isBallOut_ = false;
    isSoundWin_ = false;

    idleRandNum_ = 0;

    strings_.clear();
    winVoice_.clear();

    //ここから下はランダムに呼び出す
    strings_.push_back("voice_uwa");
    strings_.push_back("voice_aa");
    strings_.push_back("voice_ite");
    strings_.push_back("voice_u");
    strings_.push_back("voice_ugu");
    strings_.push_back("voice_kuso_high");
    strings_.push_back("voice_kuso_low");
 

    //winVoice_.push_back("voice_checkmate");
    winVoice_.push_back("voice_iityoushi");
    winVoice_.push_back("voice_iikanzi");
    //winVoice_.push_back("voice_ikke"); 
    winVoice_.push_back("voice_sugoi");
    winVoice_.push_back("voice_mazide");
}

void PlayerGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{


    if (timer_ == 0.0f) {
        No::SoundPlay("voice_iq", 1.0f, false);
    }
    timer_ += deltaTime;
    timer_ = fmodf(timer_, 3.0f);


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

    auto normalEnemyView = registry.View <
        NormalEnemyTag,
        DeathFlag>();

    bool isEnemyDead = false;

    for (auto normalEnemyEntity : normalEnemyView)
    {
        auto* deathFlag = registry.GetComponent<DeathFlag>(normalEnemyEntity);
        if (deathFlag->isDead) {
            //もし敵に当たったら
    
                isEnemyDead = true;
                break;
            
        }
    }

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        No::AnimatorComponent,
        PlayerGirlTag>();

    for (auto entity : view)
    {
      
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);
        auto* animation = registry.GetComponent<No::AnimatorComponent>(entity);

        if (timer_ <= 2.5f) {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face.png");
        } else {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face2.png");
        }

        if (isEnemyDead) {
            //勝ってるとき
            if (!isSoundWin_) {
                int randomSound = rand() % winVoice_.size();
                No::SoundEffectPlay(winVoice_[randomSound], 1.0f);

                animation->currentAnimation = rand() % 3 + 4;
                material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face2.png");

            }
     
            isSoundWin_ = true;
  
        } else if(isOut){

            if (!isBallOut_) {

         
           
                animation->currentAnimation = rand() % 2 + 8;
                if (animation->currentAnimation == 8) {
                    No::SoundPlay(strings_[0], 1.0f, false);
                } else {
                    int randNum = rand() % (strings_.size()-1)+1;
                    No::SoundPlay(strings_[randNum], 1.0f, false);
                }

                material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face2.png");

                isBallOut_ = true;

            }
            voiceTimer_ += deltaTime;

            if (voiceTimer_ >= 1.0f) {
                voiceTimer_ = 0.0f;
            }
      
        } else {
            voiceTimer_ = 0.0f;

            if (animation->time + deltaTime >= animation->animation[animation->currentAnimation].duration) {
                isSoundWin_ = false;
                isBallOut_ = false;
                do {
                    idleRandNum_ = rand() % 4;

                } while (animation->currentAnimation == idleRandNum_);
                animation->currentAnimation = idleRandNum_;
            }
        }


     


#ifdef USE_IMGUI
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
        std::string imGuiName = "playerGirlTag";
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
