#pragma once
#include "../CommandManager.h"
#include "engine/Functions/ECS/Registry.h"

#include "externals/nlohmann/json.hpp"

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
	struct Item {
		ECS::Entity entity = ECS::INVALID_ENTITY;
		nlohmann::json snapshotJson;
		int parentIndexInBatch = -1; // 親もバッチに含まれている場合のitems_内インデックス。含まれなければ-1
	};

	ECS::Registry& registry_;
	std::vector<Item> items_;
	bool exists_ = true; // 生成済み
};

}
}