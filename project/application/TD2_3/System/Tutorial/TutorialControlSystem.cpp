#include "TutorialControlSystem.h"
#include"engine/Math/Easing.h"
#include "application/TD2_3/tag.h"
using namespace NoEngine;
using namespace Easing;
void TutorialControlSystem::Update(No::Registry& registry, float deltaTime)
{
    timer_ += deltaTime;
    float firstSpriteTime = kMoveSpriteStartDuration_ + kMoveSpriteEndDuration_ + kShowTime_;
    if (timer_ <= firstSpriteTime) {

        if (timer_ <= kMoveSpriteStartDuration_ || timer_ >= kMoveSpriteStartDuration_ + kShowTime_) {
            paddleShowTimer_ += deltaTime;    
        }

    } else {
 
        if (timer_ <= firstSpriteTime + kBallPongSpriteStartDuration_ || timer_ > kBallPongSpriteEndDuration_ + kShowTime_) {
            ballPongShowTimer_ += deltaTime;
            ballPongShowTimer_ = std::clamp(ballPongShowTimer_, 0.0f, kBallPongSpriteStartDuration_ + kBallPongSpriteEndDuration_);
        }

    }

    auto spriteView = registry.View<
        No::Transform2DComponent, 
        No::SpriteComponent, 
        TutorialSpriteTag>();

    for (auto entity : spriteView) {

        auto* a = registry.GetComponent<No::Transform2DComponent>(entity);
        auto* sp = registry.GetComponent<No::SpriteComponent>(entity);

        if (sp->name == "stickL") {

            float startPos = 0.0f;
            float endPos = 0.0f;
            float time = 0.0f;

            if (paddleShowTimer_ <= kMoveSpriteStartDuration_) {
               startPos = kStartPosX_ + a->scale.x * 0.5f;
               endPos = kMiddlePosX_ - a->scale.x * 0.5f;
               time = paddleShowTimer_ / kMoveSpriteStartDuration_;  
            } else if(paddleShowTimer_ <= kMoveSpriteStartDuration_+paddleShowTimer_){
                startPos = kMiddlePosX_ + a->scale.x * 0.5f;
                endPos = kEndPosX_ - a->scale.x * 0.5f;
                time = (paddleShowTimer_ - kMoveSpriteStartDuration_)/ kMoveSpriteEndDuration_;
            }

            time = std::clamp(time, 0.0f, 1.0f);
            a->translate.x = EaseInExpo(startPos, endPos, time);
        }

        if (sp->name == "cursorMove") {

        }

        if (sp->name == "gameAButton2") {

        }

        if (sp->name == "cursorPush") {

        }

        if (sp->name == "paddleRound") {

        }

        if (sp->name == "throwBall") {

        }

        // ImGui 編集
#ifdef USE_IMGUI
        std::string imGuiName = sp->name.empty()
            ? "TutorialSprite" + std::to_string(entity)
            : sp->name;

        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat2("translate", &a->translate.x, 1.f);
        ImGui::DragFloat2("scale", &a->scale.x, 0.1f);
        ImGui::DragFloat("rotate", &a->rotation, 0.04f);

        ImGui::DragFloat2("pivot", &sp->pivot.x, 0.05f, 0.f, 1.f);
        ImGui::Checkbox("flipX", &sp->flipX);
        ImGui::Checkbox("flipY", &sp->flipY);
        ImGui::DragFloat4("uv", &sp->uv.x, 0.05f);
        ImGui::End();
#endif
    }
#ifdef USE_IMGUI

    ImGui::Begin("TutorialTimer");
    ImGui::SliderFloat("timer", &timer_, 0.0f,1.0f);
    ImGui::End();
#endif
}
