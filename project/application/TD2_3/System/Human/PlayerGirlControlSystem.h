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

    std::vector<std::string> strings_;
    std::vector<std::string> winVoice_;

    bool isBallOut_ = false;

    bool isSoundWin_ = false;

    uint32_t idleRandNum_ = 0;

};

