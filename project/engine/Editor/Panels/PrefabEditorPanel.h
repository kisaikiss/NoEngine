#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
namespace Editor {

class EffectPreviewPanel;

// AssetBrowser等から「編集」要求を受け取り、Prefab本体を専用RegistryのEntityとしてロードして
// ComponentUIで編集させ、保存時にPrefabファイルへ書き戻すウィンドウ。
class PrefabEditorPanel {
public:
	// 編集中の切り替え・終了要求は、Drawで入力確定と未保存確認を行ってから実行する。
	void Begin(const std::string& prefabPath);
	void End();
	void Draw(ECS::Registry& sceneRegistry, EffectPreviewPanel& effectPreview);

	bool IsEditing() const { return prefabEntity_ != ECS::INVALID_ENTITY; }
	bool HasUnsavedChanges() const;

private:
	enum class PendingAction { None, Close, Switch };
	void Open(std::string prefabPath);
	void Clear();
	void CompletePendingAction();
	bool SaveAndApply(ECS::Registry& sceneRegistry);
	// シーンとは独立した編集データ。Undo履歴にも所有権を渡して参照切れを防ぐ。
	std::shared_ptr<ECS::Registry> editingRegistry_;
	ECS::Entity prefabEntity_ = ECS::INVALID_ENTITY;
	std::string prefabPath_;
	nlohmann::json savedJson_;
	nlohmann::json diskJson_;
	PendingAction pendingAction_ = PendingAction::None;
	std::string pendingPath_;
	std::string errorMessage_;
};

}
}
