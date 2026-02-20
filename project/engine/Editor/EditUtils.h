#pragma once
#include "Reflection.h"
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace Editor {
struct EditTag {
	std::string name;
};
}

void DrawComponentUI(ECS::Registry& registry, ECS::Entity e);
void DrawFieldUI(const FieldInfo& field, void* ptr);

}