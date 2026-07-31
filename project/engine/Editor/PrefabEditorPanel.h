#pragma once
#include "engine/Functions/ECS/Registry.h"
#include <string>

namespace NoEngine {
namespace Editor {

// AssetBrowser等から「編集」要求を受け取り、Prefab本体を一時Entityとしてロードして
// ComponentUIで編集させ、保存時にPrefabファイルへ書き戻すウィンドウ。
class PrefabEditorPanel {
public:
	void Begin(ECS::Registry& registry, const std::string& prefabPath);
	void End(ECS::Registry& registry);
	void Draw(ECS::Registry& registry);

	bool IsEditing() const { return prefabEntity_ != ECS::INVALID_ENTITY; }
	ECS::Entity GetEditingEntity() const { return prefabEntity_; }

private:
	ECS::Entity prefabEntity_ = ECS::INVALID_ENTITY;
	std::string prefabPath_;
};

}
}
