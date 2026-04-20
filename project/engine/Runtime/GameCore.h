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
	virtual void Update(ComputeContext& ctx, float deltaT) {
		if (auto event = GetRegistry().PollEvent<Event::SceneChangeEvent>()) {
			ChangeScene(event->nextScene, true);
		}

		UpdateScene(ctx, deltaT);
	};

	/// <summary>
	/// ゲームを閉じるかどうか取得します。
	/// </summary>
	/// <returns>true : ゲームを閉じる、false : ゲームを閉じない</returns>
	virtual bool Exit();

	/// <summary>
	/// 秘匿されたメンバ変数であるECSレジストリの参照を取得します。
	/// </summary>
	/// <returns>ECSレジストリ</returns>
	ECS::Registry& GetRegistry() { return *sceneManager_->GetRegistry(); }

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
	/// <summary>
	/// シーンを登録します。
	/// </summary>
	/// <param name="name">シーン変更時などに使用する名前を設定します。</param>
	/// <param name="factory">シーンを生成する関数を登録します。(std::make_unique<>())</param>
	void RegisterScene(const std::string& name, Scene::SceneManager::SceneFactory factory) { sceneManager_->RegisterScene(name, factory); }

	/// <summary>
	/// シーンを変更します。
	/// </summary>
	/// <param name="name">変更先のシーン名</param>
	/// <param name="immediate">遷移演出(ture : 演出なし、false : 演出あり)</param>
	void ChangeScene(const std::string& name, bool immediate = true) { sceneManager_->ChangeScene(name,immediate); }

	/// <summary>
	/// シーンの更新を行います
	/// </summary>
	/// <param name="ctx">コンピュートシェーダー用のコマンドリストラッパークラス</param>
	/// <param name="deltaTime">前フレームからの経過時間</param>
	void UpdateScene(ComputeContext& ctx, float deltaTime) { sceneManager_->Update(ctx, deltaTime); }

	/// <summary>
	/// シーンマネージャーをリセットします。
	/// </summary>
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

/// <summary>
/// FPSなどのパフォーマンスをImGuiで表示します。
/// </summary>
/// <param name="deltaTime">前フレームからの経過時間</param>
void DrawPerformance(float deltaTime);
};
}
