#pragma once
#include "engine/Runtime/GameCore.h"

class Game : public NoEngine::GameCore::IGameApp {
public:
	/// <summary>
	/// ゲームアプリケーションの初期化を行います。
	/// </summary>
	void Startup(void) override;

	/// <summary>
	/// ゲームアプリケーションの終了処理を行います。
	/// </summary>
	/// <param name=""></param>
	void Cleanup(void) override {};

	/// <summary>
	/// 更新処理を行います。フレームごとに1回呼び出されます。
	/// </summary>
	/// <param name="deltaT">経過時間</param>
	void Update(float deltaT) override;
private:
	NoEngine::ECS::Entity entity_;
};

