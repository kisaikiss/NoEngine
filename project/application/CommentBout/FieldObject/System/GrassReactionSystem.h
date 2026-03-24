#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 草オブジェクトの衝突反応を処理するFieldObject層System。
/// </summary>
class GrassReactionSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
