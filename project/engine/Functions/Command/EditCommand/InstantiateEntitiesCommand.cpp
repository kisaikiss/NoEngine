#include "stdafx.h"
#include "InstantiateEntitiesCommand.h"

namespace NoEngine {
namespace Command {

InstantiateEntitiesCommand::InstantiateEntitiesCommand(ECS::Registry& registry, const std::vector<ECS::Entity>& entities)
	: ICommand("InstantiateEntities"), registry_(registry) {
	items_ = CaptureEntityBatch(registry_, entities);
}

void InstantiateEntitiesCommand::Execute() {
	if (exists_) return;

	RestoreEntityBatch(registry_, items_);

	exists_ = true;
}

void InstantiateEntitiesCommand::Undo() {
	if (!exists_) return;

	for (auto& item : items_) {
		registry_.DestroyEntity(item.entity);
	}

	exists_ = false;
}

}
}