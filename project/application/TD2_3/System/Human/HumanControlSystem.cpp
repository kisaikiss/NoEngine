#include "HumanControlSystem.h"
#include "../../Component/PhaseComponent.h"
#include"engine/Functions/Renderer/Primitive.h"
#include"engine/Math/Types/Calculations/QuaternionCalculations.h"
#include"engine/Math/Easing.h"
#include "../../tag.h"
using namespace NoEngine;
using namespace No;
using namespace MathCalculations;
HumanControlSystem::HumanControlSystem()
{
    timer_ = kDuration_;
    isChangePhase_ = false;
    angle_ = 0.0f;
    startAngle_ = 0.0f;
}

void HumanControlSystem::Update(No::Registry& registry, float deltaTime)
{
    auto view = registry.View<
        No::TransformComponent,
        EnemyHumanTag>();

    auto phaseView = registry.View<PhaseComponent>();

    for (auto entity : phaseView) {
        auto* phase = registry.GetComponent<PhaseComponent>(entity);

        switch (phase->phase) {
        case Phase::ONE:

            break;
        case Phase::TWO:

            if (!isChangePhase_) {
                isChangePhase_ = true;
                timer_ = kDuration_;
                startAngle_ = angle_;
                startAngle_ = fmod(startAngle_, PI * 2.0f);
            }
            break;

        default:
            break;
        }
    }


    for (auto entity : view) {

        auto* transform = registry.GetComponent<No::TransformComponent>(entity);

        auto batGirlView = registry.View<
            No::TransformComponent,
            BatGirlTag>();

        for (auto batGirlEntity : batGirlView) {
            registry.GetComponent<No::TransformComponent>(batGirlEntity)->parent = transform;
        }

        auto ChefView = registry.View<
            No::TransformComponent,
            ChefTag>();

        for (auto chefEntity : ChefView) {
            registry.GetComponent<No::TransformComponent>(chefEntity)->parent = transform;
        }

        if (isChangePhase_ && timer_ != 0.0f) {

            timer_ -= deltaTime;
            float time = timer_ / kDuration_;
            //タイマーをclamp
            timer_ = std::clamp(timer_, 0.0f, kDuration_);
            angle_ = NoEngine::Easing::EaseInOutBack(PI + startAngle_, startAngle_, time);
        }

        //if (timer_ == 0.0f) {
        //    isChangePhase_ = false;
        //}

        q_.FromAxisAngle(Vector3::UP, angle_);

        transform->rotation = q_;

#ifdef USE_IMGUI
        ImGui::Begin("HumanSystem");
        ImGui::SliderFloat3("translate", &transform->translate.x, -100.0f, 100.0f);
        ImGui::SliderFloat4("rotation", &transform->rotation.x, -100.0f, 100.0f);
        ImGui::SliderFloat("angle", &angle_, -3.14f, 3.14f);
        ImGui::Checkbox("isChangePase", &isChangePhase_);
        if (ImGui::Button("ChangePase")) {
            isChangePhase_ = true;
            timer_ = kDuration_;
            startAngle_ = angle_;
            startAngle_ = fmod(startAngle_, PI * 2.0f);
        }
        ImGui::End();

#endif
    }

}
