#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
// TransformRoutineComponentの再生を管理する。
struct MoverComponent {
	bool startOnAwake = false;
	bool active = false;
	bool initialized = false;
};
}
