#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Component/FieldObject/FieldPlacementComponent.h"
#include <unordered_map>
#include <string>

struct GameResourceComponent;

class FieldObjectEditor {
public:
    /// FieldEditorSystem から毎フレーム呼ぶ: 自動リロード + ビジュアル更新
    void Update(No::Registry& registry);

    /// フィールド配置タブの中身 (種別デフォルト設定は含まない)
    void DrawImGui(No::Registry& registry);

    /// 種別デフォルト設定タブの中身
    void DrawTypeDefaultsImGui(No::Registry& registry);

    /// EditorManager の「すべて読込/保存」から呼ぶ
    void ForceReload();
    void Save(No::Registry& registry);

    const std::string& GetLoadedStageName() const { return loadedStageName_; }

private:
    std::unordered_map<std::string, FieldObjectTypeDefinition> typeDefinitions_;
    No::Entity selectedEntity_ = No::nullEntity;
    int addTypeIndex_ = 0;
    int idCounter_ = 0;
    const No::Registry* loadedRegistry_ = nullptr;
    std::string loadedStageName_;
};
