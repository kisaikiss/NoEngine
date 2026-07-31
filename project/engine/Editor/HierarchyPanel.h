#pragma once
#include "engine/Functions/ECS/Registry.h"
#include <unordered_map>
#include <vector>
#include <set>

namespace NoEngine {
namespace Editor {

// Hierarchyウィンドウの描画・選択（単一/複数）・ドラッグ&ドロップによる親子付け替え・
// 生成/リネーム/削除を担当するパネル。
//
// 選択状態はこのクラスが内部で保持するが、「Inspectorに何を表示するか」の正は
// 常にEditSelectedTagが付いているEntity（ECS側）にある。このクラスはクリック時に
// EditSelectedTagを付け替えるだけで、他の場所（ビューポート等）から選択が変わる
// ケースについては関与しない＝呼び出し側がEditSelectedTagを見て判断すること。
class HierarchyPanel {
public:
	void Draw(ECS::Registry& registry);

	// ウィンドウ外（Deleteキー等）でEntityを削除した際に、
	// このパネルが保持している選択関連の内部状態からも取り除くために呼ぶ。
	void ClearSelectionOf(ECS::Entity e);

private:
	void DrawEntityNode(ECS::Registry& registry, ECS::Entity e, const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap);
	void HandleClick(ECS::Registry& registry, ECS::Entity e);
	void HandleDrop(ECS::Registry& registry, ECS::Entity newParent);

	std::set<ECS::Entity> selection_;                              // Hierarchy上の複数選択（ECSタグではない内部状態）
	ECS::Entity lastClickedEntity_ = ECS::INVALID_ENTITY;           // Shift+Click範囲選択の基準Entity
	ECS::Entity pendingSingleSelectEntity_ = ECS::INVALID_ENTITY;   // ドラッグに発展するか判定待ちの単一選択候補

	std::vector<ECS::Entity> hierarchyOrder_; // 毎フレーム再構築するプリオーダー順（Shift+Click範囲選択に使う）
};

}
}
