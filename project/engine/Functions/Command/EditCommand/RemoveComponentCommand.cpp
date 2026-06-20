#include "stdafx.h"
#include "RemoveComponentCommand.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/ComponentRegistry.h"

namespace NoEngine {
namespace Command {
RemoveComponentCommand::RemoveComponentCommand(ECS::Registry& registry, ECS::Entity entity, size_t typeID) : ICommand("RemoveComponent"),
registry_(registry), entity_(entity), typeID_(typeID) {
	auto& typeInfo = *ComponentRegistry::FindByTypeID(typeID_);
	void* compPtr = typeInfo.getter(registry, entity);

	for (auto& field : typeInfo.fields) {
		uint8_t* base = (uint8_t*)compPtr + field.offset;
		Editor::WriteFieldToJson(snapshotJson_, field, base);
	}
}

void RemoveComponentCommand::Execute() {
	registry_.RemoveComponent(typeID_, entity_);
}

void RemoveComponentCommand::Undo() {
	void* compPtr = registry_.AddComponent(typeID_, entity_);
	auto* typeInfo = ComponentRegistry::FindByTypeID(typeID_);
	for (auto& field : typeInfo->fields) {
		uint8_t* base = (uint8_t*)compPtr + field.offset;
		Editor::ReadFieldFromJson(snapshotJson_, field, base);
	}
}
}
}