#include "engine/Utilities/TypeIndex.h"

namespace NoEngine{
namespace ECS{
template<typename CompType>
CompType* Registry::AddComponent(const Entity entity) {
	size_t type = Utilities::TypeID<CompType>();
	if (!componentPools_[type]) {
		// コンポーネントプールを実体化します。
		componentPools_[type] = std::make_shared<ComponentPool<CompType>>();
	}
	// 追加するコンポーネントを格納するコンテナクラスを取得
	std::shared_ptr<ComponentPool<CompType>> compPool = std::static_pointer_cast<ComponentPool<CompType>>(componentPools_[type]);
	// コンポーネントを追加し取得
	CompType* resultComp = compPool->AddComponent(entity);

	// 追加したコンポーネントを返す
	return resultComp;
}

template<typename CompType>
void Registry::RemoveComponent(const Entity entity) {
	// 削除するコンポーネントを格納するコンテナクラスを取得
	std::shared_ptr<ComponentPool<CompType>> compPool = std::static_pointer_cast<ComponentPool<CompType>>(componentPools_[Utilities::TypeID<CompType>()]);
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
	auto it = componentPools_.find(Utilities::TypeID<CompType>());
	if (it == componentPools_.end()) return nullptr;

	return static_cast<ComponentPool<CompType>*>(it->second.get());
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