#include "stdafx.h"
#include "DeleteEntityCommand.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"

namespace NoEngine {
namespace Command {
DeleteEntityCommand::DeleteEntityCommand(ECS::Registry& registry, ECS::Entity entity) : ICommand("DeleteEntity"), registry_(registry), entity_(entity) {
    // Entity の全コンポーネントを JSON に保存しておく
    snapshotJson_ = Editor::SaveEntityToJson(registry_, entity_);
}

void DeleteEntityCommand::Execute() {
    registry_.DestroyEntity(entity_);
}

void DeleteEntityCommand::Undo() {
    // 再生成: 新しい Entity を作って snapshot から復元
    entity_ = registry_.GenerateEntity();
    Editor::LoadEntityFromJson(registry_, entity_, snapshotJson_);
    registry_.AddComponent<Editor::EditSelectedTag>(entity_);
}

}
}