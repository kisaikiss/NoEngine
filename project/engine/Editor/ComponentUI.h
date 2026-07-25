#pragma once
#include "Reflection.h"
#include "engine/Functions/ECS/Registry.h"
namespace NoEngine {

void DrawComponentUI(ECS::Registry& registry, ECS::Entity e);
void DrawFieldUI(ECS::Registry& registry, const FieldInfo& field, void* ptr);
}