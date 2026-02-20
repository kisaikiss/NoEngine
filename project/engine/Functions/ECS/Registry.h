#pragma once
#include "Entity.h"
#include "ComponentPool.h"
#include "EventBus.h"

namespace NoEngine {
namespace ECS {
/// <summary>
/// Entityの発行とコンポーネントとの紐づけを行うクラス
/// </summary>
class Registry final {
public:
	/// <summary>
	/// 有効なエンティティを新規作成します。
	/// </summary>
	/// <returns>新たなエンティティ</returns>
	Entity GenerateEntity();

	/// <summary>
	/// エンティティを無効化します。内部的には無効化予定のエンティティを追加するだけなので即時無力化されるわけではありません。
	/// </summary>
	/// <param name="entity">無効化したいエンティティ</param>
	void DestroyEntity(Entity entity);

	/// <summary>
	/// 無効化保留中のエンティティを一斉削除します。
	/// </summary>
	void FlushDestroy();

	/// <summary>
	/// エンティティにコンポーネントを追加します。
	/// </summary>
	/// <typeparam name="CompType">コンポーネント構造体</typeparam>
	/// <param name="entity">エンティティ</param>
	/// <returns>取り付けた構造体</returns>
	template<typename CompType>
	CompType* AddComponent(const Entity entity);

	/// <summary>
	///  エンティティのコンポーネントを取り外します。
	/// </summary>
	/// <typeparam name="CompType">取り外したいコンポーネント</typeparam>
	/// <param name="entity">コンポーネントを取り外すエンティティ</param>
	template<typename CompType>
	void RemoveComponent(const Entity entity);

	/// <summary>
	/// 指定したエンティティと関連付けられたコンポーネントのポインタを取得します。
	/// </summary>
	/// <typeparam name="CompType">取得したい種類のコンポーネント</typeparam>
	/// <param name="entity">エンティティ</param>
	/// <returns>コンポーネントのポインタ</returns>
	template<typename CompType>
	CompType* GetComponent(const Entity entity);

	/// <summary>
	/// エンティティが指定したコンポーネントと関連付けられているか判別します。
	/// </summary>
	/// <typeparam name="CompType">コンポーネント</typeparam>
	/// <param name="e">エンティティ</param>
	/// <returns>true : 関連付けられている, false : 関連付けられていない</returns>
	template<typename CompType>
	bool Has(Entity e) const;

	/// <summary>
	/// エンティティが指定したTypeIDのコンポーネントと関連付けられているか判別します。
	/// </summary>
	/// <param name="typeID">コンポーネントのTypeID</param>
	/// <param name="e">エンティティ</param>
	/// <returns>true : 関連付けられている, false : 関連付けられていない</returns>
	bool Has(size_t typeID, Entity e) const;

	/// <summary>
	/// エンティティが指定した全てのコンポーネントと関連付けられているか判別します。
	/// </summary>
	/// <typeparam name="...Components">コンポーネント。複数指定できます。</typeparam>
	/// <param name="e">エンティティ</param>
	/// <returns>true : 関連付けられている, false : 関連付けられていない</returns>
	template<typename... Components>
	bool HasAll(Entity e) const;

	/// <summary>
	/// クエリ。レジストリと基準プールを保持し、基準プールのエンティティを走査しながらHasAllをチェックします。
	/// </summary>
	/// <typeparam name="...Components">複数のコンポーネント</typeparam>
	template<typename... Components>
	class Query {
	public:
		Query(Registry& registry) : registry_(registry) {
			base_ = registry_.GetSmallestPool<Components...>();
		}

		struct Iterator {
			Registry& registry;
			IComponentPool* base;
			size_t index;

			Entity operator*() const {
				return base->Entities()[index];
			}

			Iterator& operator++() {
				do {
					index++;
				} while (index < base->Size() &&
					!registry.HasAll<Components...>(base->Entities()[index]));
				return *this;
			}

			bool operator!=(const Iterator& other) const {
				return index != other.index;
			}
		};

		Iterator begin() {
			size_t idx = 0;
			while (idx < base_->Size() &&
				!registry_.HasAll<Components...>(base_->Entities()[idx]))
				idx++;
			return { registry_, base_, idx };
		}

		Iterator end() {
			return { registry_, base_, base_->Size() };
		}

		bool Empty() {
			return !base_;
		}

		bool NoEntity() {
			if (Empty()) {
				return true;
			}

			return (base_->Size() == 0);
		}

	private:
		Registry& registry_;
		IComponentPool* base_;
	};

	/// <summary>
	/// クエリを取得します。クエリはイテレータを持っており、指定した複数のコンポーネントを持つエンティティを得てコンポーネント同士の操作をすることに使用します。
	/// </summary>
	/// <typeparam name="...Components">複数のコンポーネント</typeparam>
	/// <returns>クエリ</returns>
	template<typename... Components>
	auto View() {
		return Query<Components...>(*this);
	}

	/// <summary>
	/// イベントを追加します。
	/// </summary>
	/// <typeparam name="Event">追加するイベント構造体の種類</typeparam>
	/// <param name="e">追加するイベント</param>
	template<typename Event>
	void EmitEvent(const Event& e) {
		eventBus_.Emit(e);
	}

	/// <summary>
	/// 指定したイベントを取得します。
	/// </summary>
	/// <typeparam name="Event">イベント構造体の種類</typeparam>
	/// <returns>イベント</returns>
	template<typename Event>
	std::optional<Event> PollEvent() {
		return eventBus_.Poll<Event>();
	}


	bool Empty();
private:
	// エンティティと有効フラグを紐づけするコンテナ
	std::vector<bool> entityToActive_;
	// 再利用待ちのエンティティ
	std::vector<Entity> freeEntities_;
	// 削除待ちのエンティティ
	std::vector<Entity> pendingDestroy_;
	// イベントの集合
	EventBus eventBus_;

	// 次に生成するエンティティのID
	size_t nextID_ = 0;

	// コンポーネントをタイプ別に管理するコンテナ
	std::vector<std::unique_ptr<IComponentPool>> componentPools_;

	/// <summary>
	/// 指定した型のコンポーネントプールを取得します。
	/// </summary>
	/// <typeparam name="CompType">コンポーネント構造体</typeparam>
	/// <returns>コンポーネントプール</returns>
	template<typename CompType>
	ComponentPool<CompType>* GetPool() const;

	/// <summary>
	/// 指定した複数の型のコンポーネントプールのうち、最もエンティティ数が少ないものを取得します。
	/// </summary>
	/// <typeparam name="...Components">コンポーネント構造体、複数指定できます。</typeparam>
	/// <returns>最もエンティティ数が少ないコンポーネントプール</returns>
	template<typename... Components>
	IComponentPool* GetSmallestPool();
};
}
}

#include "Registry.hpp"
