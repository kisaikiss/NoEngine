#include "stdafx.h"
#include "EntityBatchSnapshot.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine {
namespace Command {

std::vector<EntityBatchSnapshotItem> CaptureEntityBatch(ECS::Registry& registry, const std::vector<ECS::Entity>& entities) {
	std::vector<EntityBatchSnapshotItem> items;
	items.reserve(entities.size());

	// Entity -> バッチ内でのインデックス（親子関係の復元に使う）
	std::unordered_map<ECS::Entity, int> indexOf;
	indexOf.reserve(entities.size() * 2);
	for (size_t i = 0; i < entities.size(); ++i) {
		indexOf[entities[i]] = static_cast<int>(i);
	}

	for (auto e : entities) {
		EntityBatchSnapshotItem item;
		item.entity = e;
		item.snapshotJson = Editor::SaveEntityToJson(registry, e);

		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		auto it = (tag && tag->parent != ECS::INVALID_ENTITY) ? indexOf.find(tag->parent) : indexOf.end();
		item.parentIndexInBatch = (it != indexOf.end()) ? it->second : -1;

		items.push_back(std::move(item));
	}
	return items;
}

void RestoreEntityBatch(ECS::Registry& registry, std::vector<EntityBatchSnapshotItem>& items) {
	// 1) まず全Entityを再生成してsnapshotから復元する
	for (auto& item : items) {
		item.entity = registry.GenerateEntity();
		Editor::LoadEntityFromJson(registry, item.entity, item.snapshotJson);
	}

	// 2) バッチ内に親がいたEntityだけ、新しいIDで親子関係を張り直す
	//    （バッチ外に親を持つEntityは、そのEntityのIDがそのまま生きているので触らない）
	for (auto& item : items) {
		if (item.parentIndexInBatch < 0) continue;
		auto* tag = registry.GetComponent<Editor::EditTag>(item.entity);
		if (tag) {
			tag->parent = items[static_cast<size_t>(item.parentIndexInBatch)].entity;
		}
	}

	// 3) 元々選択されていたであろう代表として、最後のEntityにだけ選択タグを付ける
	if (!items.empty()) {
		registry.AddComponent<Editor::EditSelectedTag>(items.back().entity);
	}
}

}
}
