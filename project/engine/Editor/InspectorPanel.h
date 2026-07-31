#pragma once
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace Editor {

// Inspectorウィンドウを描画する。EditSelectedTagが付いているEntity（＝代表選択）を
// 自分でRegistryから探すので、呼び出し側は誰が選択中かを渡す必要はない。
class InspectorPanel {
public:
	void Draw(ECS::Registry& registry);

private:
	void DrawAddComponentMenu(ECS::Registry& registry, ECS::Entity entity);

	char addComponentFilter_[128] = {};
};

}
}
