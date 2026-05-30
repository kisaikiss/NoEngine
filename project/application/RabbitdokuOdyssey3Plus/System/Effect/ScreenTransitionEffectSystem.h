#pragma once
#include "engine/NoEngine.h"
#include "../../Game/RabbitdokuSerializer.h"
class ScreenTransitionEffectSystem :
    public No::ISystem {
public:
    ScreenTransitionEffectSystem() { SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void TransitionIn(No::Registry& registry, float deltaTime);
    void TransitionOut(No::Registry& registry, float deltaTime);
    std::string nextScene_;
    SaveData saveData_;
};