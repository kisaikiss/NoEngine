#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// チュートリアルステージのオブジェクトの更新処理を行うメソッド。
/// </summary>
class TutorialObjectSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

};