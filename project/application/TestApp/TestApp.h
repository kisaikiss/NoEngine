#pragma once
#include "engine/NoEngine.h"

class TestApp : public No::IGameApp {
public:
	/// <summary>
	/// ゲームアプリケーションの初期化を行います。
	/// </summary>
	void Startup(void) override;
};

