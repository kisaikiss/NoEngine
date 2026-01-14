#pragma once
#include "engine/Functions/ECS/Registry.h"
namespace NoEngine {
namespace GameCore {
/// <summary>
/// ゲームアプリケーションを作成する際はこのクラスを継承してアプリケーションクラスを作成します。
/// </summary>
class IGameApp {
	friend int RunApplication(std::unique_ptr<IGameApp> game);
public:
	virtual ~IGameApp() = default;

	/// <summary>
	/// ゲームアプリケーションの初期化を行います。
	/// </summary>
	virtual void Startup(void) = 0;

	/// <summary>
	/// ゲームアプリケーションの終了処理を行います。
	/// </summary>
	/// <param name=""></param>
	virtual void Cleanup(void) = 0;

	/// <summary>
	/// 更新処理を行います。フレームごとに1回呼び出されます。
	/// </summary>
	/// <param name="deltaT">経過時間</param>
	virtual void Update(float deltaT) = 0;

	virtual void RenderScene(void) {};

protected:
	ECS::Registry& GetRegistry() { return *registry_; }
private:
	ECS::Registry* registry_;
	void SetRegistry(ECS::Registry* registry) { registry_ = registry; }
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
