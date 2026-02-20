#pragma once

enum class TutorialPhase {
    START,
    MIDDLE,
    END,
};

struct TutorialSpriteComponent
{
    float startPos = 0.0f;
    float endPos = 0.0f;
    float startTimer = 0.0f;
    float middleTimer = 0.0f;   
    float endTimer = 0.0f;
    TutorialPhase phase = TutorialPhase::START;
};