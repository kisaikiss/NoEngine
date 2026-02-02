#include "Registry.h"

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
	pendingDestroy_.push_back(entity);
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

bool Registry::Empty() {
	return entityToActive_.empty();
}

}
}
