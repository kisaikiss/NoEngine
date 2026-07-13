#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/System/SystemManager.h"
#include "engine/Functions/ECS/Component/PauseComponent.h"
#include "SceneNameComponent.h"

namespace NoEngine {
namespace Scene {

/// <summary>
/// シーン基底クラス
/// </summary>
class IScene {
	friend class SceneManager;
public:
	IScene();
	virtual ~IScene() = default;

	/// <summary>
	/// エントリ時に呼び出される仮想メソッド。派生クラスでオーバーライドして状態遷移時の処理（BGM再生など）を行えます。
	/// </summary>
	virtual void OnEnter() {}

	/// <summary>
	/// 終了時に呼び出される仮想メソッド。既定の実装は何もしません。派生クラスでオーバーライドして終了処理を実装してください。
	/// </summary>
	virtual void OnExit() {}

	/// <summary>
	/// シーンで使うエンティティ、コンポーネント、システムをセットします。
	/// </summary>
	virtual void Setup() = 0;

	/// <summary>
	/// レジストリ内のすべてのシステムを、指定した経過時間で更新します。
	/// </summary>
	/// <param name="deltaTime">前フレームからの経過時間。システムの更新に使用されます。</param>
	void Update(ComputeContext& ctx, float deltaTime);

	ECS::Registry* GetRegistry() { return registry_.get(); }
protected:
	template<typename T>
	T* AddSystem(std::unique_ptr<T> system) { return systemManager_->AddSystem(std::move(system)); }

	/// <summary>
	/// Systemではない更新処理。主にテスト用に使用します。
	/// </summary>
	virtual void NotSystemUpdate(){}
private:
	void SetName(const std::string& name) {
		auto view = registry_->View<SceneNameComponent>();
		for (auto entity : view) {
			auto* scene = registry_->GetComponent<SceneNameComponent>(entity);
			scene->name = name;
		}
	}
	std::unique_ptr<ECS::Registry> registry_;
	std::unique_ptr<ECS::SystemManager> systemManager_;
};

std::string GetCurrentSceneName(ECS::Registry& registry);
}
}

