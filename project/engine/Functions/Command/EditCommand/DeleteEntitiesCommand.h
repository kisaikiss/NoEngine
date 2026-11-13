#pragma once
#include "../CommandManager.h"
#include "EntityBatchSnapshot.h"

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
	ECS::Registry& registry_;
	std::vector<EntityBatchSnapshotItem> items_;
};

}
}