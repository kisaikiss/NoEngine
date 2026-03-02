#pragma once
#include "Entity.h"
namespace NoEngine {
namespace ECS {
/// <summary>
/// コンポーネントプールを同じコンテナで管理するための基底クラス
/// </summary>
class IComponentPool {
public:
	virtual ~IComponentPool() = default;
	virtual void RemoveIfExists(Entity entity) = 0;
	virtual void* GetVoidPointerComponent(Entity entity) = 0;
	virtual size_t Size() const = 0;
	virtual bool Has(Entity entity) const = 0;
	virtual const std::vector<Entity>& Entities() const = 0;
};

/// <summary>
/// コンポーネントを管理するコンテナクラス
/// </summary>
/// <typeparam name="CompType">コンポーネント構造体</typeparam>
template<typename CompType>
class ComponentPool : public IComponentPool {
public:
	/// <summary>
	/// コンストラクタでコンテナのメモリを確保します。
	/// </summary>
	/// <param name="size">メモリサイズ</param>
	ComponentPool() {}

	/// <summary>
	/// エンティティにコンポーネントを追加します。
	/// </summary>
	/// <param name="entity">コンポーネントを追加したいエンティティ</param>
	/// <returns>追加したコンポーネントのポインタ</returns>
	inline CompType* AddComponent(const Entity entity) {
		if (entityToIndex_.size() <= entity) {
			entityToIndex_.resize(entity + 1, -1);
		}
		entityToIndex_[entity] = static_cast<int32_t>(components_.size());
		components_.emplace_back(CompType());
		indexToEntity_.emplace_back(entity);

		return &components_[entityToIndex_[entity]];
	}

	/// <summary>
	/// エンティティのコンポーネントを取り外します。
	/// </summary>
	/// <param name="entity">コンポーネントを取り外したいエンティティ</param>
	inline void RemoveComponent(Entity entity) {
		int32_t idx = entityToIndex_[entity];
		if (idx < 0) return;

		int32_t last = static_cast<int32_t>(components_.size()) - 1;
		Entity lastEntity = indexToEntity_[last];

		// swap remove
		components_[idx] = components_[last];
		indexToEntity_[idx] = lastEntity;
		entityToIndex_[lastEntity] = idx;

		components_.pop_back();
		indexToEntity_.pop_back();
		entityToIndex_[entity] = -1;

	}

	/// <summary>
	/// 指定したエンティティがコンポーネントを持っているかどうか取得します。
	/// </summary>
	/// <param name="entity">エンティティ</param>
	/// <returns>コンポーネントを持っているかどうか(true : 持っている, false : 持っていない)</returns>
	bool Has(Entity entity) const override {
		return entity < entityToIndex_.size() && entityToIndex_[entity] >= 0;
	}

	/// <summary>
	/// コンポーネントを取得します。
	/// </summary>
	/// <param name="entity">コンポーネントと関連付けられたエンティティ</param>
	/// <returns>コンポーネントのポインタ</returns>
	inline CompType* GetComponent(const Entity entity) noexcept {
		// エンティティが有効なら
		int32_t index = entityToIndex_[entity];
		if (index >= 0) {
			return &components_[index];
		}

		return nullptr;
	}

	inline void* GetVoidPointerComponent(Entity entity) override {
		// エンティティが有効なら
		int32_t index = entityToIndex_[entity];
		if (index >= 0) {
			return static_cast<void*>(&components_[index]);
		}

		return nullptr;
	}

	void RemoveIfExists(Entity entity) override {
		if (Has(entity)) {
			RemoveComponent(entity);
		}
	}

	size_t Size() const override { return components_.size(); }
	const std::vector<Entity>& Entities() const override { return indexToEntity_; }


private:
	// コンポーネントのインスタンスを管理するコンテナ
	std::vector<CompType> components_;
	// コンポーネントのインデックスを管理するコンテナ
	std::vector<int32_t> entityToIndex_;
	std::vector<Entity> indexToEntity_;
};

}
}
