#include "stdafx.h"
#include "TransformRoutineSystem.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Math/Easing.h"

namespace NoEngine {
namespace ECS {
using namespace Component;
void TransformRoutineSystem::Update(Registry& registry, float deltaTime) {
    auto view = registry.View<TransformRoutineComponent, TransformComponent>();
    for (auto e : view) {
        auto* routine = registry.GetComponent<TransformRoutineComponent>(e);
        auto* transform = registry.GetComponent<TransformComponent>(e);

        if (!routine->playing || routine->keyframes.size() < 2) continue;

        int nextIndex = (routine->currentIndex + 1) % routine->keyframes.size();
        const auto& from = routine->keyframes[routine->currentIndex];
        const auto& to = routine->keyframes[nextIndex];

        routine->elapsed += deltaTime * routine->playbackSpeed;
        float rawT = std::clamp(routine->elapsed / to.duration, 0.0f, 1.0f);
        float time = ApplyEasing(to.easing, rawT);

        switch (routine->interpolation) {
        case InterpolationType::Linear:
            transform->translate = Easing::Lerp(from.translate, to.translate, time);
            break;
        case InterpolationType::CatmullRom: 
            break;
        case InterpolationType::Bezier:
            break;
        case InterpolationType::Step:
            transform->translate = (rawT >= 1.0f) ? to.translate : from.translate;
            break;
        }

        transform->rotation.Slerp(from.rotation, to.rotation, time);
        transform->scale = Easing::Lerp(from.scale, to.scale, time);

        if (rawT >= 1.0f) {
            routine->elapsed = 0.0f;
            routine->currentIndex = nextIndex;
            if (!routine->loop && routine->currentIndex == 0) {
                routine->playing = false;
            }
        }
    }
}
}
}