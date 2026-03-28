#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 吹き出し（SpeechBubble）をボスへ飛ばし、到達時にダメージを与えるシステム
/// </summary>
class SpeechBubbleToBossSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
