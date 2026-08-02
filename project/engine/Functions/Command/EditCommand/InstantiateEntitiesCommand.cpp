#include "stdafx.h"
#include "InstantiateEntitiesCommand.h"

#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Editor/ReflectionMacros.h"

#include <unordered_map>

namespace NoEngine {
namespace Command {

InstantiateEntitiesCommand::InstantiateEntitiesCommand(ECS::Registry& registry, const std::vector<ECS::Entity>& entities)
	: ICommand("InstantiateEntities"), registry_(registry) {
	items_.reserve(entities.size());

	// Entity -> バッチ内でのインデックス（親子関係の復元に使う）
	std::unordered_map<ECS::Entity, int> indexOf;
	indexOf.reserve(entities.size() * 2);
	for (size_t i = 0; i < entities.size(); ++i) {
		indexOf[entities[i]] = static_cast<int>(i);
	}

	for (auto e : entities) {
		Item item;
		item.entity = e;
		item.snapshotJson = Editor::SaveEntityToJson(registry_, e);

		auto* tag = registry_.GetComponent<Editor::EditTag>(e);
		auto it = (tag && tag->parent != ECS::INVALID_ENTITY) ? indexOf.find(tag->parent) : indexOf.end();
		item.parentIndexInBatch = (it != indexOf.end()) ? it->second : -1;

		items_.push_back(std::move(item));
	}
}

void InstantiateEntitiesCommand::Execute() {
	if (exists_) return;

	// 1) まず全Entityを再生成してsnapshotから復元する
	for (auto& item : items_) {
		item.entity = registry_.GenerateEntity();
		Editor::LoadEntityFromJson(registry_, item.entity, item.snapshotJson);
	}

	// 2) バッチ内に親がいたEntityだけ、新しいIDで親子関係を張り直す
	for (auto& item : items_) {
		if (item.parentIndexInBatch < 0) continue;
		auto* tag = registry_.GetComponent<Editor::EditTag>(item.entity);
		if (tag) {
			tag->parent = items_[static_cast<size_t>(item.parentIndexInBatch)].entity;
		}
	}

	if (!items_.empty()) {
		registry_.AddComponent<Editor::EditSelectedTag>(items_.back().entity);
	}

	exists_ = true;
}

void InstantiateEntitiesCommand::Undo() {
	if (!exists_) return;

	for (auto& item : items_) {
		registry_.DestroyEntity(item.entity);
	}

	exists_ = false;
}

}
}