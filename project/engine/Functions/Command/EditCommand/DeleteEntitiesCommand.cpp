#include "stdafx.h"
#include "DeleteEntitiesCommand.h"

namespace NoEngine {
namespace Command {

DeleteEntitiesCommand::DeleteEntitiesCommand(ECS::Registry& registry, const std::vector<ECS::Entity>& entities)
	: ICommand("DeleteEntities"), registry_(registry) {
	items_ = CaptureEntityBatch(registry_, entities);
}

void DeleteEntitiesCommand::Execute() {
	for (auto& item : items_) {
		registry_.DestroyEntity(item.entity);
	}
}

void DeleteEntitiesCommand::Undo() {
	RestoreEntityBatch(registry_, items_);
}

}
}