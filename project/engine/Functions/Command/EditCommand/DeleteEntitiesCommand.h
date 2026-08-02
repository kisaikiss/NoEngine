#pragma once
#include "../CommandManager.h"
#include "engine/Functions/ECS/Registry.h"

#include "externals/nlohmann/json.hpp"

namespace NoEngine {
namespace Command {
/// <summary>
/// 複数のEntityをまとめて1回のUndo/Redoで削除・復元するためのコマンド。
/// 
/// バッチ内のEntity同士に親子関係がある場合（親も一緒に削除対象になっている場合）、
/// Undoで全Entityを再生成するとIDが変わるため、コンストラクタ時点でバッチ内インデックスを使って親子関係を記録しておき、Undo時にそのインデックスから新しいIDへ張り直す。
/// バッチに含まれない（削除されない）親を持つEntityは、そのEntityが生き残っている前提で元のIDのままにする。
/// </summary>
class DeleteEntitiesCommand : public ICommand {
public:
	DeleteEntitiesCommand(ECS::Registry& registry, const std::vector<ECS::Entity>& entities);
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
};

}
}