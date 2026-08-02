#include "EntityHierarchyUtils.h"
#include "engine/Editor/ReflectionMacros.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/Command/EditCommand/DeleteEntitiesCommand.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"

namespace NoEngine {
namespace Editor {

bool IsDescendantOf(ECS::Registry& registry, ECS::Entity ancestorCandidate, ECS::Entity entity) {
	// ancestorCandidate から親を辿って entity に到達できれば、
	// entity は ancestorCandidate の祖先（＝ancestorCandidateはentityの子孫）ということになる
	ECS::Entity current = ancestorCandidate;
	while (current != ECS::INVALID_ENTITY) {
		if (current == entity) return true;
		auto* tag = registry.GetComponent<Editor::EditTag>(current);
		current = tag ? tag->parent : ECS::INVALID_ENTITY;
	}
	return false;
}

void SetEntityParent(ECS::Registry& registry, ECS::Entity child, ECS::Entity newParent) {
	if (child == ECS::INVALID_ENTITY || child == newParent) return;

	// 自分の子孫を自分の親にしようとした場合は循環参照になるので中止する
	if (newParent != ECS::INVALID_ENTITY && IsDescendantOf(registry, newParent, child)) {
		LogWarning("EntityHierarchyUtils: cannot parent an entity to its own descendant.");
		return;
	}

	auto* tag = registry.GetComponent<Editor::EditTag>(child);
	if (!tag) return;
	tag->parent = newParent;

	// 実際のTransform階層（ワールド座標計算に使われる parent）も、
	// 型が一致する場合のみ同期する。フォルダ（Transformを持たないEntity）へ
	// 入れた場合は見た目上の階層分けのみに留まり、座標計算には影響しない。
	if (auto* childTransform = registry.GetComponent<Component::TransformComponent>(child)) {
		Component::TransformComponent* parentTransform =
			(newParent != ECS::INVALID_ENTITY) ? registry.GetComponent<Component::TransformComponent>(newParent) : nullptr;
		childTransform->parent = (newParent == ECS::INVALID_ENTITY || parentTransform) ? newParent : ECS::INVALID_ENTITY;
	} else if (auto* childTransform2D = registry.GetComponent<Component::Transform2DComponent>(child)) {
		Component::Transform2DComponent* parentTransform2D =
			(newParent != ECS::INVALID_ENTITY) ? registry.GetComponent<Component::Transform2DComponent>(newParent) : nullptr;
		childTransform2D->parent = (newParent == ECS::INVALID_ENTITY || parentTransform2D) ? newParent : ECS::INVALID_ENTITY;
	}
}

void CollectSubtreeEntities(ECS::Registry& registry, ECS::Entity root, std::vector<ECS::Entity>& outEntities) {
	outEntities.push_back(root);

	// EditTagを持つ全Entityをスキャンしてrootのparentになっているものを再帰的に集める
	// （エディタでの操作前提のためO(N)スキャンで問題ない規模を想定）
	auto view = registry.View<Editor::EditTag>();
	for (auto e : view) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag && tag->parent == root) {
			CollectSubtreeEntities(registry, e, outEntities);
		}
	}
}

void BuildHierarchyMap(
	ECS::Registry& registry,
	std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& outChildrenMap,
	std::vector<ECS::Entity>& outRoots) {
	outChildrenMap.clear();
	outRoots.clear();

	auto view = registry.View<Editor::EditTag>();
	for (auto e : view) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag->parent == ECS::INVALID_ENTITY) {
			outRoots.push_back(e);
		} else {
			outChildrenMap[tag->parent].push_back(e);
		}
	}
}

void BuildPreOrder(
	ECS::Entity e,
	const std::unordered_map<ECS::Entity, std::vector<ECS::Entity>>& childrenMap,
	std::vector<ECS::Entity>& outOrder) {
	outOrder.push_back(e);
	auto it = childrenMap.find(e);
	if (it == childrenMap.end()) return;
	for (auto child : it->second) {
		BuildPreOrder(child, childrenMap, outOrder);
	}
}

void DeleteEntitiesWithUndo(ECS::Registry& registry, const std::vector<ECS::Entity>& entities) {
	if (entities.empty()) return;

	// 1回のコマンドにまとめて記録することで、1回のUndoで全Entity（子孫含む）を復元できるようにする。
	// Entity毎に別々のコマンドを積むと、元に戻すのに同じ回数だけUndoを押す必要があって不便なため。
	Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::DeleteEntitiesCommand>(registry, entities));
	for (auto e : entities) {
		registry.DestroyEntity(e);
	}
}

}
}