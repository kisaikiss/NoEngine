#pragma once
#include "../CommandManager.h"
#include "EntityBatchSnapshot.h"

namespace NoEngine {
namespace Command {

/// <summary>
/// 複数のEntityをまとめて1回のUndo/Redoで生成・破棄するためのコマンド。
/// </summary>
class InstantiateEntitiesCommand : public ICommand {
public:
	InstantiateEntitiesCommand(ECS::Registry& registry, const std::vector<ECS::Entity>& entities);
	void Execute() override;
	void Undo() override;

private:
	ECS::Registry& registry_;
	std::vector<EntityBatchSnapshotItem> items_;
	bool exists_ = true; // 生成済み
};

}
}