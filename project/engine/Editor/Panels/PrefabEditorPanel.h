#pragma once
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
namespace Editor {

// AssetBrowser等から「編集」要求を受け取り、Prefab本体を専用RegistryのEntityとしてロードして
// ComponentUIで編集させ、保存時にPrefabファイルへ書き戻すウィンドウ。
class PrefabEditorPanel {
public:
	void Begin(const std::string& prefabPath);
	void End();
	void Draw(ECS::Registry& sceneRegistry);

	bool IsEditing() const { return prefabEntity_ != ECS::INVALID_ENTITY; }

private:
	// シーンとは独立した編集データ。Undo履歴にも所有権を渡して参照切れを防ぐ。
	std::shared_ptr<ECS::Registry> editingRegistry_;
	ECS::Entity prefabEntity_ = ECS::INVALID_ENTITY;
	std::string prefabPath_;
};

}
}
