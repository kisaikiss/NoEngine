#include "PlayerGirlControlSystem.h"
#include "../../Component/BallStateComponent.h"
#include "../../tag.h"

PlayerGirlControlSystem::PlayerGirlControlSystem()
{    //strings_.push_back("voice_checkmate");
    timer_ = 0.0f;
    voiceTimer_ = 0.0f;
    isSoundBallOut_ = false;
    strings_.clear();

    //ここから下はランダムに呼び出す
    strings_.push_back("voice_aa");
    strings_.push_back("voice_ite");
    strings_.push_back("voice_kuso_high");
    strings_.push_back("voice_kuso_low");
    strings_.push_back("voice_u");
    strings_.push_back("voice_ugu");
    strings_.push_back("voice_uwa");
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
    if (isOut) {
        if (!isSoundBallOut_) {
            int randNum = rand() % strings_.size();
            No::SoundPlay(strings_[randNum], 1.0f, false);
            isSoundBallOut_ = true;
        }
        voiceTimer_ += deltaTime;

        if (voiceTimer_ >= 1.0f) {
            voiceTimer_ = 0.0f;
        }

    } else {
        voiceTimer_ = 0.0f;
        isSoundBallOut_ = false;
    }

    auto view = registry.View<
        No::TransformComponent,
        No::MaterialComponent,
        PlayerGirlTag>();



    //もし敵に当たったら
   // No::SoundPlay("voice_iityoushi", 1.0f, false);

    for (auto entity : view)
    {
      
        auto* material = registry.GetComponent<No::MaterialComponent>(entity);

        if (timer_ <= 2.5f) {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face.png");
        } else {
            material->materials[1].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/playerGirl/face2.png");
        }
#ifdef USE_IMGUI
        auto* transform = registry.GetComponent<No::TransformComponent>(entity);
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
