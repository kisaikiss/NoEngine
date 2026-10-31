#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"
#include "engine/Functions/ECS/Entity.h"

namespace NoEngine::Component {
// Colliderを持つEntityに付け、接触開始時にイベントを発行する。
struct TriggerComponent {
	bool oneShot = true;      // trueなら最初の接触開始だけイベントを出す。
	bool activated = false;   // oneShot Triggerが既に発火したかを記録する。
	bool wasTouching = false; // TriggerSystemが接触開始と接触継続を区別する前フレーム状態。
	ECS::Entity lastInstigator = ECS::INVALID_ENTITY; // 接触が終わった相手をExitイベントへ渡すための実行時状態。
};
}
