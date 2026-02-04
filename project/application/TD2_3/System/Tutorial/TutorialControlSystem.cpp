#include "TutorialControlSystem.h"
#include"engine/Math/Easing.h"
#include "application/TD2_3/tag.h"
#include "../../Component/TutorialSpriteComponent.h"
using namespace NoEngine;
using namespace Easing;
void TutorialControlSystem::Update(No::Registry& registry, float deltaTime)
{
    timer_ += deltaTime;

    if (timer_ <= 2.0f) {
        return;
    }


    auto spriteView = registry.View<
        No::Transform2DComponent,
        No::SpriteComponent,
        TutorialSpriteTag>();

    for (auto entity : spriteView) {

        auto* transform = registry.GetComponent<No::Transform2DComponent>(entity);
        auto* sp = registry.GetComponent<No::SpriteComponent>(entity);
        auto* tutorialComp = registry.GetComponent<TutorialSpriteComponent>(entity);

        if (timer_ <= 3.0f) {
            transform->translate.x = kStartPosX_ + transform->scale.x;

        }  else if (timer_ <= 10.0f) {
            if (sp->name == "stickL") {
                Vector2 offset = transform->scale * 0.5f;
                MoveSprite(transform, tutorialComp, 
                    {-offset.x-240.0f+32.0f,0.0f
            }, deltaTime);
            }

            if (sp->name == "cursorMove") {
                Vector2 offset = transform->scale;
                MoveSprite(transform, tutorialComp,
                    { -offset.x- 240.0f, 0.0f
                    }, deltaTime);
            }
            if (sp->name == "paddleRound") {
                MoveSprite(transform, tutorialComp, {0.0f,0.0f}, deltaTime);
            }
        } else {
            if (sp->name == "gameAButton2") {
                Vector2 offset = transform->scale * 0.5f;
                MoveSprite(transform, tutorialComp,
                    { -offset.x - 240.0f+32.0f, 0.0f
                    }, deltaTime);
            }

            if (sp->name == "cursorPush") {
                Vector2 offset = transform->scale;
                MoveSprite(transform, tutorialComp,
                    { -offset.x  - 240.0f, 0.0f
                    }, deltaTime);
            }

            if (sp->name == "throwBall") {
                MoveSprite(transform, tutorialComp, {0.0f,0.0f}, deltaTime);
            }
        }
   
   
        // ImGui 編集
#ifdef USE_IMGUI
        std::string imGuiName = sp->name.empty()
            ? "TutorialSprite" + std::to_string(entity)
            : sp->name;

        ImGui::Begin(imGuiName.c_str());
        ImGui::DragFloat2("translate", &transform->translate.x, 1.f);
        ImGui::DragFloat2("scale", &transform->scale.x, 0.1f);
        ImGui::DragFloat("rotate", &transform->rotation, 0.04f);
        ImGui::SliderFloat("startTimer", &tutorialComp->startTimer, 0.0f, 1.0f);
        ImGui::SliderFloat("middleTimer", &tutorialComp->middleTimer, 0.0f, 1.0f);
        ImGui::SliderFloat("endTimer", &tutorialComp->endTimer, 0.0f, 1.0f);
        ImGui::DragFloat2("pivot", &sp->pivot.x, 0.05f, 0.f, 1.f);
        ImGui::Checkbox("flipX", &sp->flipX);
        ImGui::Checkbox("flipY", &sp->flipY);
        ImGui::DragFloat4("uv", &sp->uv.x, 0.05f);
        ImGui::End();
#endif
    }
#ifdef USE_IMGUI

    ImGui::Begin("TutorialTimer");
    ImGui::SliderFloat("timer", &timer_, 0.0f, 1.0f);
    ImGui::End();
#endif
}

void TutorialControlSystem::MoveSprite(No::Transform2DComponent* transform, TutorialSpriteComponent* tutorial, const Vector2& offset, float deltaTime)
{
 
    switch (tutorial->phase)
    {
    case  TutorialPhase::START: {
        if (tutorial->startTimer <= kFirstStartTime) {
            tutorial->startTimer += deltaTime;
        } else {
            tutorial->phase = TutorialPhase::MIDDLE;
        }

        float time = tutorial->startTimer / kFirstStartTime;
        time = std::clamp(time, 0.0f, 1.0f);

        tutorial->startPos = kStartPosX_ + transform->scale.x * 0.5f+offset.x;
        tutorial->endPos = kMiddlePosX_ + offset.x;
        transform->translate.x = EaseInOutBack(tutorial->startPos, tutorial->endPos, time);

    }

      break;
    case  TutorialPhase::MIDDLE: {
        if (tutorial->middleTimer <= kFirstShowTime) {
            tutorial->middleTimer += deltaTime;
        } else {
            tutorial->phase = TutorialPhase::END;
        }

        transform->translate.x = Lerp(transform->translate.x, kMiddlePosX_ +offset.x, 0.05f);

    }

 break;
    case  TutorialPhase::END: {

        if (tutorial->endTimer <= kFirstEndTime) {
            tutorial->endTimer += deltaTime;
        }
        tutorial->startPos = kMiddlePosX_ + offset.x;
        float offsetX = 0.0f;
        if (offset.x < 0.0f) {
            offsetX = -offset.x;
        } else {
            offsetX = offset.x;
        }
        tutorial->endPos = kEndPosX_ - transform->scale.x * 0.5f- offsetX;

        float time = tutorial->endTimer / kFirstEndTime;
        time = std::clamp(time, 0.0f, 1.0f);
        transform->translate.x = EaseInOutBack(tutorial->startPos, tutorial->endPos, time);
    }

                            break;
    default:
        break;

    }
}
