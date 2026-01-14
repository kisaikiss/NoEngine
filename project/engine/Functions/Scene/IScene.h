#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/System/SystemManager.h"
#include "engine/Functions/Camera/Camera.h"

namespace NoEngine {
namespace Scene {

/// <summary>
/// シーン基底クラス
/// </summary>
class IScene {
public:
	IScene() : registry_(std::make_unique<ECS::Registry>()), systemManager_(std::make_unique<ECS::SystemManager>()) {}
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
	void Update(float deltaTime) { NotSystemUpdate(); systemManager_->UpdateAll(*registry_, deltaTime); }

	ECS::Registry* GetRegistry() { return registry_.get(); }

	CameraBase* GetCamera() { return useCamera_; }

protected:
	void AddSystem(std::unique_ptr<ECS::ISystem> system) { systemManager_->AddSystem(std::move(system)); }

	void SetCamera(CameraBase* camera) { useCamera_ = camera; }

	/// <summary>
	/// Systemではない更新処理。主にテスト用に使用します。
	/// </summary>
	virtual void NotSystemUpdate(){}
private:
	std::unique_ptr<ECS::Registry> registry_;
	std::unique_ptr<ECS::SystemManager> systemManager_;

	// ToDo : カメラもECSで扱うようにすべきです。
	CameraBase* useCamera_ = nullptr;
};
}
}

