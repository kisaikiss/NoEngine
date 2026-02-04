#include "TutorialControlSystem.h"
#include"engine/Math/Easing.h"
#include "application/TD2_3/tag.h"
using namespace NoEngine;
using namespace Easing;
void TutorialControlSystem::Update(No::Registry& registry, float deltaTime)
{
    timer_ += deltaTime;
    
    if (timer_ >= 0.0f) {
        paddleShowTimer_ += deltaTime;
    } else if (timer_ >= 5.0f) {
        ballPongShowTimer_ += deltaTime;
    }	

    auto spriteView = registry.View<
        No::Transform2DComponent, 
        No::SpriteComponent, 
        TutorialSpriteTag>();

    for (auto entity : spriteView) {

        auto* a = registry.GetComponent<No::Transform2DComponent>(entity);
        auto* sp = registry.GetComponent<No::SpriteComponent>(entity);

        if (sp->name == "stickL") {
            float startPos = kStartPosX_ + a->scale.x * 0.5f;
            float endPos = kMiddlePosX_ - a->scale.x * 0.5f;
            a->translate.x = EaseInExpo(startPos, endPos,timer_);
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
