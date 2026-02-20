#include "PlayerGirlControlSystem.h"
#include "../../Component/BallStateComponent.h"
#include "../../Component/PhaseComponent.h"
#include "../../Component/PlayerStatusComponent.h"
#include "../../tag.h"

PlayerGirlControlSystem::PlayerGirlControlSystem()
{

    //PLAYER_VOICE
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_aa.mp3", "voice_aa");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_ite.mp3", "voice_ite");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_kuso_low.mp3", "voice_kuso_low");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_u.mp3", "voice_u");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_ugu.mp3", "voice_ugu");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_uwa.mp3", "voice_uwa");

    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_iikanzi.mp3", "voice_iikanzi");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_iityoushi.mp3", "voice_iityoushi");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_ikke.mp3", "voice_ikke");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_sugoi.mp3", "voice_sugoi");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_mazide.mp3", "voice_mazide");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_checkmate.mp3", "voice_checkmate");
    No::SoundLoad(L"resources/game/td_2304//Audio/Voice/voice_tabemono.mp3", "voice_tabemono");

    //strings_.push_back("voice_checkmate");
    blinkTimer_ = 0.0f;
    voiceTimer_ = 0.0f;
    idleActionTimer_ = 0.0f;

    isBallOut_ = false;
    isSoundWin_ = false;
  
    idleRandNum_ = 0;

    strings_.clear();
    winVoice_.clear();


    //ここから下はランダムに呼び出す
    //strings_.push_back("voice_tabemono");
    strings_.push_back("voice_aa");
    strings_.push_back("voice_ite");
    strings_.push_back("voice_u");
    strings_.push_back("voice_ugu");
    strings_.push_back("voice_kuso_low");

    //winVoice_.push_back("voice_checkmate");
        //winVoice_.push_back("voice_ikke"); 

    winVoice_.push_back("voice_iityoushi");
    winVoice_.push_back("voice_iikanzi");
    winVoice_.push_back("voice_sugoi");
    winVoice_.push_back("voice_mazide");

}

void PlayerGirlControlSystem::Update(No::Registry& registry, float deltaTime)
{

    auto phaseView = registry.View<PhaseComponent>();
    PhaseComponent* phase = nullptr;
    for (auto entity : phaseView) {
        phase = registry.GetComponent<PhaseComponent>(entity);
    }

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
        BatTag,
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

    auto whiteRadishView = registry.View <
        WhiteRadishTag,
        DeathFlag>();

    for (auto whiteRadishEntity : whiteRadishView)
    {
        auto* deathFlag = registry.GetComponent<DeathFlag>(whiteRadishEntity);
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

        if (!PlayerStatusComponent::isGameClear && !PlayerStatusComponent::isGameOver) {
            if (isEnemyDead) {
                //勝ってるとき
                if (!isBallOut_ && !isSoundWin_) {
                    int randomSound = rand() % winVoice_.size();
                    No::SoundEffectPlay(winVoice_[randomSound], 1.0f);

                    animation->currentAnimation = rand() % 2 + 5;
                    int faceRand = rand() % 2;
                    if (faceRand == 0) {
                        material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face_surprise.png");

                    } else if (faceRand == 1) {
                        material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face_wink.png");

                    } else {
                        material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face2.png");

                    }

                }

                isSoundWin_ = true;

            } else if (isOut) {


                if (!isBallOut_ && !isSoundWin_) {

                    animation->currentAnimation = rand() % 2 + 7;
                    if (animation->currentAnimation == 7) {
                        No::SoundPlay("voice_uwa", 1.0f, false);
                    } else {
                        if (phase->phase == Phase::TWO) {
                            int randNum = rand() % strings_.size() + 1;

                            if (randNum != strings_.size()) {
                                No::SoundPlay(strings_[randNum], 1.0f, false);
                            } else {
                                No::SoundPlay("voice_tabemono", 1.0f, false);
                            }


                        } else {
                            int randNum = rand() % strings_.size();

                            No::SoundPlay(strings_[randNum], 1.0f, false);

                        }




                    }

                    material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face_sad.png");

                    isBallOut_ = true;

                }
                voiceTimer_ += deltaTime;

                if (voiceTimer_ >= 1.0f) {
                    voiceTimer_ = 0.0f;
                }

            } else {
                voiceTimer_ = 0.0f;

                //アクションタイマーを足す
                idleActionTimer_ += deltaTime;

                if (animation->time + deltaTime >= animation->animation[animation->currentAnimation].duration) {

                    isSoundWin_ = false;
                    isBallOut_ = false;

                    if (idleActionTimer_ >= 20.0f) {

                        do {
                            idleRandNum_ = rand() % 4 + 1;
                        } while (animation->currentAnimation == idleRandNum_);
                        animation->currentAnimation = idleRandNum_;
                        idleActionTimer_ = 0.0f;

                    } else {
                        animation->currentAnimation = 0;
                    }
                }

                if (!isSoundWin_ && !isBallOut_) {
                    blinkTimer_ += deltaTime;
                    blinkTimer_ = fmodf(blinkTimer_, 3.0f);

                    if (blinkTimer_ <= 2.75f) {
                        material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face.png");
                    } else {
                        material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face_blink.png");
                    }

                }



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
