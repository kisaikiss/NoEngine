#pragma once

#include "engine/NoEngine.h"
#include<vector>
class PlayerGirlControlSystem :
    public No::ISystem
{
public:
    PlayerGirlControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
private:
    float timer_ = 0.0f;
    float voiceTimer_ = 0.0f;
    float winVoiceTimer_ = 0.0f;
    std::vector<std::string> strings_;
    std::vector<std::string> winVoice_;

    bool isSoundBallOut_ = false;
    bool isSoundWin_ = false;
};

