#pragma once
#include "engine/NoEngine.h"

class SampleScene : public No::IScene {
public:
	void Setup() override;
private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};
	void NotSystemUpdate() override;

	// グリッド初期化
	void InitializeGrid(No::Registry& registry);

	// プレイヤー初期化
	void InitializePlayer(No::Registry& registry);

	// ライト初期化
	void InitializeLight(No::Registry& registry);


	void DestroyGameObject();
};