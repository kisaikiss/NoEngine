#pragma once
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace ECS { class SystemManager; }
namespace Editor {

/// <summary>■ > || ボタンを描画し、PlayModeControllerを操作するパネル</summary>
class GameControllerPanel {
public:
	void Draw(ECS::Registry& registry, ECS::SystemManager* systems = nullptr);
};

}
}
