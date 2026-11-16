#pragma once
#include "../Reflection/Reflection.h"
#include "engine/Functions/ECS/Registry.h"
namespace NoEngine {

// 個別フィールドの履歴を破棄し、変更確定の有無を返す。呼び出し側で値の履歴を記録する。
bool CaptureFieldEditCommands(const std::function<void()>& draw);
void DrawComponentUI(ECS::Registry& registry, ECS::Entity e);
void DrawFieldUI(ECS::Registry& registry, ECS::Entity e, const FieldInfo& field, void* ptr);
}