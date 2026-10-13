#pragma once
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace Editor {

/// <summary>■ > || ボタンを描画し、PlayModeControllerを操作するパネル</summary>
class GameControllerPanel {
public:
	void Draw(ECS::Registry& registry);
};

}
}