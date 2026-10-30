#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
// Scene Entityから独立したギミックグラフJSONへの参照。
struct GimmickGraphComponent {
	std::string graphPath;
};
}
