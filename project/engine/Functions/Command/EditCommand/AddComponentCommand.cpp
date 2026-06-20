#include "stdafx.h"
#include "AddComponentCommand.h"
#include "engine/Editor/ComponentRegistry.h"

namespace NoEngine {
namespace Command {
AddComponentCommand::AddComponentCommand(ECS::Registry& registry, ECS::Entity entity, size_t typeID) : ICommand("AddComponent"),
registry_(registry), entity_(entity), typeID_(typeID) {}

void AddComponentCommand::Execute() {
	registry_.AddComponent(typeID_, entity_);
}

void AddComponentCommand::Undo() {
	registry_.RemoveComponent(typeID_, entity_);
}
}
}