#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Component/Effect/EffectEmitterComponent.h"
namespace NoEngine {
namespace ECS {

class EffectEmitSystem :
    public ISystem {
public:
    EffectEmitSystem() { SetStopInGameStop(false); SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;
private:
    friend class Registry;

    struct Playback {
        Component::EffectPreset preset;
        Math::Vector3 worldPosition;
        Math::Quaternion rotation = Math::Quaternion::IDENTITY;
        double timeToNextBurst = 0.0;
        uint32_t burstsRemaining = 0;
    };

    std::vector<Playback> playbacks_;
};

}
}
