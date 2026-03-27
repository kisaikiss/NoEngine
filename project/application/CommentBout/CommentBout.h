#pragma once
#include "engine/NoEngine.h"


/// <summary>
/// シューティングゲーム
/// </summary>
class CommentBout : public No::IGameApp {

	/// <summary>
	/// ゲームアプリケーションの初期化を行います。
	/// </summary>
	void Startup(void) override;

public:

	void Update(NoEngine::ComputeContext& ctx, float deltaT) {
		if (auto event = GetRegistry().PollEvent<NoEngine::Event::SceneChangeEvent>()) {
			ChangeScene(event->nextScene, false);
		}

		UpdateScene(ctx, deltaT);
	};

};