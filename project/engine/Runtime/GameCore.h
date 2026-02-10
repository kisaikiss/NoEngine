#pragma once
#include "engine/Functions/Scene/SceneManager.h"
#include "engine/Functions/Renderer/RenderPass/RenderPassScheduler.h"
#include "engine/Functions/ECS/Event/SceneChangeEvent.h"

namespace NoEngine {
namespace GameCore {
/// <summary>
/// ゲームアプリケーションを作成する際はこのクラスを継承してアプリケーションクラスを作成します。
/// </summary>
class IGameApp {
public:
	IGameApp() : sceneManager_(std::make_unique<Scene::SceneManager>()){}
	virtual ~IGameApp() = default;

	/// <summary>
	/// ゲームアプリケーションの初期化を行います。
	/// </summary>
	virtual void Startup(void) = 0;

	/// <summary>
	/// ゲームアプリケーションの終了処理を行います。
	/// </summary>
	/// <param name=""></param>
	virtual void Cleanup(void) { ShutdownSceneManager(); }

	/// <summary>
	/// 更新処理を行います。フレームごとに1回呼び出されます。
	/// </summary>
	/// <param name="deltaT">経過時間</param>
	virtual void Update(float deltaT) {
		if (auto event = GetRegistry().PollEvent<Event::SceneChangeEvent>()) {
			ChangeScene(event->nextScene, true);
		}

		UpdateScene(deltaT);
	};

	virtual void RenderScene(void) {};

	virtual bool Exit();

	ECS::Registry& GetRegistry() { return *sceneManager_->GetRegistry(); }

	CameraBase* GetCamera() { return sceneManager_->GetCamera(); }

	/// <summary>
	/// カスタム RenderPass を追加します。IGameApp から呼び出してください。
	/// </summary>
	void AddRenderPass(std::unique_ptr<Render::RenderPass>&& pass)
	{
		if (renderPassScheduler_) 
			renderPassScheduler_->AddRenderPass(std::move(pass));
	}

	/// <summary>
	/// フレームワーク内部用: RunApplication がスケジューラを設定します。
	/// </summary>
	void SetRenderPassScheduler(Render::RenderPassScheduler* scheduler) { renderPassScheduler_ = scheduler; }

protected:
	void RegisterScene(const std::string& name, Scene::SceneManager::SceneFactory factory) { sceneManager_->RegisterScene(name, factory); }

	void ChangeScene(const std::string& name, bool immediate = true) { sceneManager_->ChangeScene(name,immediate); }

	void UpdateScene(float deltaTime) { sceneManager_->Update(deltaTime); }

	void ShutdownSceneManager() { sceneManager_.reset(); }

private:
	std::unique_ptr<Scene::SceneManager> sceneManager_;
	Render::RenderPassScheduler* renderPassScheduler_ = nullptr;
};

/// <summary>
/// アプリケーションを走らせます。
/// </summary>
/// <param name="game">走らせるアプリケーション</param>
/// <returns>正常に終了すると0を返します。</returns>
int RunApplication(std::unique_ptr<IGameApp> game);

/// <summary>
/// エンジンを初期化します。
/// </summary>
void EngineInitialize();

/// <summary>
/// エンジンの終了処理を行います。
/// </summary>
void EngineFinalize();

/// <summary>
/// 前回の呼び出しからの経過時間を計算して返します。
/// </summary>
/// <returns>経過時間を秒単位で表す float 値。</returns>
float CalculateDeltaTime();
};
}
