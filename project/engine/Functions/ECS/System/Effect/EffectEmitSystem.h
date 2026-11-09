#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/Effect/EffectEmitterComponent.h"
#include <random>
namespace NoEngine {
namespace ECS {

class EffectEmitSystem :
    public ISystem {
public:
    struct EmissionStats {
        size_t spawned = 0;
        uint64_t dropped = 0;
    };

    EffectEmitSystem() { SetStopInGameStop(false); SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;
    EmissionStats UpdatePreview(Registry& registry, float deltaTime, size_t availableSlots, std::mt19937& random);
private:
    friend class Registry;

    struct Playback {
        Component::EffectPreset preset;
        Math::Vector3 worldPosition;
        Math::Quaternion rotation = Math::Quaternion::IDENTITY;
        double timeToNextBurst = 0.0;
        uint32_t burstsRemaining = 0;
    };

    EmissionStats UpdatePlaybacks(Registry& registry, float deltaTime, size_t availableSlots, std::mt19937* random);
    std::vector<Playback> playbacks_;
};

}
}
