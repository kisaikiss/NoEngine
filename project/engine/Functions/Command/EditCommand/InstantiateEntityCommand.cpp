#include "stdafx.h"
#include "InstantiateEntityCommand.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"

namespace NoEngine {
namespace Command {
InstantiateEntityCommand::InstantiateEntityCommand(ECS::Registry& registry, ECS::Entity entity) : ICommand("InstantiateEntity"), registry_(registry), entity_(entity) {
    // 保存用: 生成元プリセットを探すか、Entity の全コンポーネントを JSON に保存しておく
    snapshotJson_ = Editor::SaveEntityToJson(registry_, entity_);
}

void InstantiateEntityCommand::Execute() {
    if (exists_) return;
    // 再生成: 新しい Entity を作って snapshot から復元
    entity_ = registry_.GenerateEntity();
    Editor::LoadEntityFromJson(registry_, entity_, snapshotJson_);
    registry_.AddComponent<Editor::EditSelectedTag>(entity_);
    exists_ = true;
}
void InstantiateEntityCommand::Undo() {
    if (!exists_) return;
    registry_.DestroyEntity(entity_);
    exists_ = false;
}
}
}