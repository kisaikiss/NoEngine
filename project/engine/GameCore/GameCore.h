#pragma once
#include "../Utilities/PassKey/PassKeys.h"
namespace NoEngine {
namespace GameCore {
/// <summary>
/// ゲームアプリケーションを作成する際はこのクラスを継承してアプリケーションクラスを作成します。
/// </summary>
class IGameApp {
public:
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

	virtual void RenderScene(void) = 0;

private:

};


int RunApplication(AllowAccessOnlyFromWinMain);
void EngineInitialize();
void EngineFinalize();
};
}
