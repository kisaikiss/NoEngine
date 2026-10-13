#pragma once
#include "engine/Functions/ECS/Registry.h"
#include <unordered_map>
#include <vector>

// EditTag::parent を辿るだけの純粋なECS階層ロジック。ImGuiに依存しないため、
// HierarchyPanel / EntityClipboard の両方、および将来追加されるツールからも再利用できる。
namespace NoEngine {
namespace Editor {

// child の親を newParent に変更する。
// Transform / Transform2D の parent も、型が一致する場合のみ同期する
// （フォルダのようにTransformを持たないEntityへ入れた場合は見た目の階層分けのみになり、
// 　ワールド座標の計算には影響しない）。
// newParent が child の子孫の場合（循環参照になる場合）は何もしない。
void SetEntityParent(ECS::Registry& registry, ECS::Entity child, ECS::Entity newParent);

// ancestorCandidate が entity の子孫かどうか
// （＝ancestorCandidateから親を辿ってentityに到達できるか）を調べる。循環参照防止に使う。
bool IsDescendantOf(ECS::Registry& registry, ECS::Entity ancestorCandidate, ECS::Entity entity);

// root自身とその子孫を全て再帰的に集める。
// EditTagを持つ全EntityをO(N)スキャンして親子を辿るため、エディタ操作の規模を想定している。
void CollectSubtreeEntities(ECS::Registry& registry, ECS::Entity root, std::vector<ECS::Entity>& outEntities);

// EditTag::parent を元に「親Entity -> 子Entity一覧」のマップと、親を持たないルート一覧を構築する。
void BuildHierarchyMap(
	ECS::Registry& registry,
	std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& outChildrenMap,
	std::vector<ECS::Entity>& outRoots);

// childrenMap を使って、e を根とするプリオーダー順の一覧を outOrder に積む（Shift+Click範囲選択などに使う）。
void BuildPreOrder(
	ECS::Entity e,
	const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap,
	std::vector<ECS::Entity>& outOrder);

// entities を破棄する。破棄する各Entityごとに個別のUndo用コマンド(DeleteEntityCommand)を
// 記録してから破棄するため、Undo/Redoで途中のEntity（子孫など）だけが復元されない、
// という事態を防げる。「削除対象一覧を集める処理」と「Undo可能な形で破棄する処理」を
// 呼び出し側ごとに書き直さないよう、削除経路（Deleteキー／右クリックメニュー等）は
// 必ずこの関数を経由すること。
void DeleteEntitiesWithUndo(ECS::Registry& registry, const std::vector<ECS::Entity>& entities);

}
}