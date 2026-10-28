#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
// Colliderを持つEntityに付け、接触開始時にイベントを発行する。
struct TriggerComponent {
	bool oneShot = true;
	bool activated = false;
	bool wasTouching = false;
};
}
