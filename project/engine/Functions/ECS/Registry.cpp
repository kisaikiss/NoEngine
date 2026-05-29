#include "Registry.h"
#include "engine/Editor/ComponentRegistry.h"

namespace NoEngine {
namespace ECS {
Entity Registry::GenerateEntity() {
	Entity newEntity;
	// リサイクル待ちエンティティがあればそこから1つ取り出す
	if (freeEntities_.size()) {
		newEntity = freeEntities_.back();
		freeEntities_.pop_back();
	} else { // 無ければ新規発行
		newEntity = ++nextID_;
	}
	// エンティティを有効にする
	if (entityToActive_.size() <= newEntity) {
		entityToActive_.resize(newEntity + 1, false);
	}
	entityToActive_[newEntity] = true;
	// 生成したエンティティを返す
	return newEntity;
}

void Registry::DestroyEntity(Entity entity) {
	pendingDestroy_.insert(entity);
}

void Registry::FlushDestroy() {
	for (auto entity : pendingDestroy_) {
		entityToActive_[entity] = false;
		freeEntities_.emplace_back(entity);
		for (auto& pool : componentPools_) {
			if (pool)
				pool->RemoveIfExists(entity);
		}
	}
	pendingDestroy_.clear();
}

void* Registry::AddComponent(const size_t& typeId, const Entity& entity) {
	// ComponentRegistry から TypeInfo を取得
	TypeInfo* info = ComponentRegistry::FindByTypeID(typeId);
	if (!info) return nullptr;

	// テンプレート AddComponent を呼ぶ
	return info->adder(*this, entity);
}

void Registry::RemoveComponent(const size_t& typeId, const Entity& entity) {
	if (typeId >= componentPools_.size()) return;
	if (!componentPools_[typeId]) return;
	componentPools_[typeId]->RemoveIfExists(entity);
}

void* Registry::GetComponent(const size_t& typeId, const Entity& entity) {
	if (typeId >= componentPools_.size()) return nullptr;
	if (!componentPools_[typeId] || !componentPools_[typeId]->Has(entity)) return nullptr;
	return componentPools_[typeId]->GetVoidPointerComponent(entity);
}

bool Registry::Has(size_t typeID, Entity e) const {
	return componentPools_[typeID] && componentPools_[typeID]->Has(e);
}

bool Registry::Empty() {
	return entityToActive_.empty();
}

}
}
