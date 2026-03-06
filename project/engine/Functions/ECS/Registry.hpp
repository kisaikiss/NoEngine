#include "engine/Utilities/TypeIndex.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"

namespace NoEngine{
namespace ECS{
template<typename CompType>
CompType* Registry::AddComponentInternal(const Entity entity) {
	size_t type = Utilities::TypeID<CompType>();
	if (componentPools_.size() <= type) {
		componentPools_.resize(type + 1);
		componentPools_[type] = std::make_unique<ComponentPool<CompType>>();
	}

	if (componentPools_[type] == nullptr) {
		componentPools_[type] = std::make_unique<ComponentPool<CompType>>();
	}

	// 追加するコンポーネントを格納するコンテナクラスを取得
	ComponentPool<CompType>* compPool = static_cast<ComponentPool<CompType>*>(componentPools_[type].get());
	// コンポーネントを追加し取得
	CompType* resultComp = compPool->AddComponent(entity);

	// 追加したコンポーネントを返す
	return resultComp;
}

template<typename CompType>
CompType* Registry::AddComponent(const Entity entity) {
	return AddComponentInternal<CompType>(entity);
}

/// <summary>
/// コンポーネント追加のカメラタグについての特殊化。アクティブカメラタグはただ一つとなるように動作します。
/// </summary>
/// <param name="e">エンティティ</param>
/// <returns>カメラタグ</returns>
template<>
inline Component::ActiveCameraTag* Registry::AddComponent<Component::ActiveCameraTag>(const Entity e) {
	// すでに付いているなら何もしない
	if (Has<Component::ActiveCameraTag>(e))
		return GetComponent<Component::ActiveCameraTag>(e);

	// 他の Entity から ActiveCameraTag を外す
	auto view = View<Component::ActiveCameraTag>();
	for (Entity other : view) {
		if (other != e && Has<Component::ActiveCameraTag>(other)) {
			RemoveComponent<Component::ActiveCameraTag>(other);
		}
	}

	// この Entity に付ける
	return AddComponentInternal<Component::ActiveCameraTag>(e);
}

template<typename CompType>
void Registry::RemoveComponent(const Entity entity) {
	// 削除するコンポーネントを格納するコンテナクラスを取得
	ComponentPool<CompType>* compPool = static_cast<ComponentPool<CompType>*>(componentPools_[Utilities::TypeID<CompType>()].get());
	// コンポーネントをエンティティから取り外す
	compPool->RemoveComponent(entity);
}

template<typename CompType>
CompType* Registry::GetComponent(const Entity entity) {
	if (auto* s = GetPool<CompType>()) {
		return s->GetComponent(entity);
	}
	return nullptr;
}

template<typename CompType>
bool  Registry::Has(Entity e) const {
	// 指定した型のコンポーネントプールがあるか判別します。
	if (auto s = GetPool<CompType>()) {
		// コンポーネントプールが存在するならそれが指定したエンティティと関連付けられているか判別し返します。
		return s->Has(e);
	}
	return false;
}

template<typename... Components>
bool Registry::HasAll(Entity e) const {
	return (Has<Components>(e) && ...);
}

template<typename CompType>
ComponentPool<CompType>* Registry::GetPool() const {
	if (componentPools_.size() <= Utilities::TypeID<CompType>()) return nullptr;

	return static_cast<ComponentPool<CompType>*>(componentPools_[Utilities::TypeID<CompType>()].get());
}

template<typename... Components>
IComponentPool* Registry::GetSmallestPool() {
	IComponentPool* smallest = nullptr;
	size_t minSize = SIZE_MAX;

	(([&] {
		auto* s = GetPool<Components>();
		if (s && s->Size() < minSize) {
			minSize = s->Size();
			smallest = s;
		}
		}()), ...);


	return smallest;

}
}
}