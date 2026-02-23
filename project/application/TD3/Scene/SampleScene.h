#pragma once
#include "engine/NoEngine.h"
#include "../MapData/StageData.h"

class SampleScene : public No::IScene {
public:
	void Setup() override;
private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};
	void NotSystemUpdate() override;

	/// <summary>
	/// グリッド初期化
	/// ConnectionMapData を受け取り、各ノードを ECS に登録する。
	/// </summary>
	void InitializeGrid(No::Registry& registry, const MapData::ConnectionMapData& mapData);

	/// <summary>
	/// プレイヤー初期化
	/// グリッド座標を引数で受け取り、ワールド座標への変換はPlayerMovementSystem::UpdateTransform に任せる。
	/// </summary>
	void InitializePlayer(No::Registry& registry, int startX, int startY);

	/// <summary>
	/// 敵初期化
	/// </summary>
	void InitializeEnemy(No::Registry& registry, int startX, int startY);

	/// <summary>
	/// ライト初期化
	/// </summary>
	void InitializeLight(No::Registry& registry);

	/// <summary>
	/// イテレータ安全版の一括削除
	/// </summary>
	void DestroyGameObject();
};