#pragma once
#include "engine/NoEngine.h"

class ClockworksDisease : public No::IGameApp {
public:
	/// <summary>
	/// ゲームアプリケーションの初期化を行います。
	/// </summary>
	void Startup(void) override;
};

