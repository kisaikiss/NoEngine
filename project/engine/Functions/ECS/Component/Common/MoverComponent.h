#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
// TransformRoutineComponentの再生開始状態を管理する。実際の補間はTransformRoutineSystemが行う。
struct MoverComponent {
	bool startOnAwake = false; // 初回更新時にルートを自動再生する。
	bool active = false;       // 起動イベントを受けてMoverが有効になった状態。
	bool initialized = false;  // startOnAwakeの初期化を二重に行わないための内部状態。
};
}
