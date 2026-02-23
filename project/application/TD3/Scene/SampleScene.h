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

	/// <summary>
	/// プレイヤー初期化
	/// グリッド座標を引数で受け取り、ワールド座標への変換はPlayerMovementSystem::UpdateTransform に任せる。
	/// </summary>
	/// <param name="startX">プレイヤーの初期グリッドX座標</param>
	/// <param name="startY">プレイヤーの初期グリッドY座標</param>
	void InitializePlayer(No::Registry& registry, int startX, int startY);

	//敵初期化
	void InitializeEnemy(No::Registry& registry, int startX, int startY);
	
	// ライト初期化
	void InitializeLight(No::Registry& registry);

	/// イテレータ安全版の一括削除
	void DestroyGameObject();
};