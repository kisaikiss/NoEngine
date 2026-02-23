#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerComponent.h"
#include "../Component/GridCellComponent.h"

/// <summary>
/// プレイヤー移動システム
/// 入力処理・移動判定・ノード到達処理・モデル回転・isMovingフラグ更新を担当する。
/// </summary>
class PlayerMovementSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	// エッジ終点近傍での先行入力を有効にする閾値 [0.0 ~ 1.0]
	static constexpr float NEAR_END_THRESHOLD = 0.75f;

	// ========== 状態別の入力・移動処理 ==========

	/// <summary>
	/// ノード上にいるときの入力処理
	/// </summary>
	void HandleNodeInput(
		PlayerComponent* player,
		No::Registry& registry
	);

	/// <summary>
	/// エッジ移動中の処理
	/// </summary>
	void HandleEdgeMovement(
		PlayerComponent* player,
		float deltaTime,
		No::Registry& registry
	);

	/// <summary>
	/// エッジ途中で停止中のときの処理
	/// </summary>
	void HandleStoppedOnEdge(
		PlayerComponent* player,
		No::Registry& registry
	);

	/// <summary>
	/// 入力履歴の更新処理
	/// </summary>
	void UpdateRecentInputs(
		PlayerComponent* player,
		float deltaTime
	);

	/// <summary>
	/// ノード到達時の処理
	/// </summary>
	void OnReachNode(
		PlayerComponent* player,
		No::Registry& registry
	);

	/// <summary>
	/// 指定方向への移動を開始する
	/// </summary>
	void StartMovement(
		PlayerComponent* player,
		Direction dir,
		No::Registry& registry
	);

	/// <summary>
	/// エッジ途中で停止する
	/// </summary>
	void StopMovement(PlayerComponent* player);

	// ========== 終点近傍チェック ==========

	/// <summary>
	/// ターゲットノードで有効な方向キーが現在押されているか判定する
	/// </summary>
	bool HasValidNearEndInput(
		PlayerComponent* player,
		No::Registry& registry,
		Direction futureLastDir
	);

	// ========== 判定ヘルパー ==========

	/// <summary>
	/// 指定ノードから指定方向に移動できるか判定する。
	/// 接続チェックと後退禁止ルールを考慮する（行き止まりの後退は例外許可）。
	/// この関数はゲーム固有ロジックを含むため GridUtils には移さない。
	/// Enemy も同じルールを使うため、Stage3 で共有化を検討する。
	/// </summary>
	bool CanMoveInDirection(
		No::Registry& registry,
		int nodeX, int nodeY,
		Direction dir,
		Direction lastDir
	);

	// ========== Transform 更新 ==========

	/// <summary>
	/// プレイヤーのワールド座標を計算する
	/// GridUtils::GridToWorld を使用するためスケール変更に対応済み
	/// </summary>
	No::Vector3 CalculateWorldPosition(
		const PlayerComponent* player
	);

	/// <summary>
	/// プレイヤーの Transform（位置・回転）を更新する
	/// </summary>
	void UpdateTransform(
		PlayerComponent* player,
		No::TransformComponent* transform
	);

	/// <summary>
	/// 移動方向に対応する回転クオータニオンを返す
	/// </summary>
	NoEngine::Math::Quaternion CalcDirectionRotation(Direction dir);

	// ========== 行き止まりチェック ==========

	/// <summary>
	/// 現在ノードが行き止まりかを判定し isAtDeadEnd フラグを更新する
	/// </summary>
	void CheckDeadEnd(
		PlayerComponent* player,
		No::Registry& registry
	);

	// ========== 交差点検出と弾薬配置 ==========

	bool IsIntersection(const GridCellComponent* cell);

	void HandleIntersection(
		PlayerComponent* player,
		No::Registry& registry
	);

	bool HasAmmoAtPosition(
		No::Registry& registry,
		int gridX,
		int gridY
	);

	void CreateAmmoItem(
		No::Registry& registry,
		int gridX,
		int gridY
	);

	void EnableAmmoPickup(
		No::Registry& registry,
		int gridX,
		int gridY
	);

	// ========== デバッグ UI ==========
#ifdef USE_IMGUI
	void DebugUI(PlayerComponent* player);
	const char* DirectionToString(Direction dir);
	const char* StateToString(PlayerState state);
#endif
};