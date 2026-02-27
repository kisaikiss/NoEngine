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
	// この値以上の progress のとき、ターゲットノードで有効な方向キーを押すだけで自動前進が許可される
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
	/// （方向は返さない。どの方向に進むかは OnReachNode の recentInputs に任せる）
	/// </summary>
	/// <param name="futureLastDir">ターゲットノード到達後の lastDirection（現在の進行方向）</param>
	/// <returns>有効なキーが1つ以上押されていれば true </returns>
	bool HasValidNearEndInput(
		PlayerComponent* player,
		No::Registry& registry,
		Direction futureLastDir
	);

	// ========== Transform 更新 ==========

	/// <summary>
	/// プレイヤーのワールド座標を計算する
	/// OnNode のときはノード座標、エッジ上は線形補間で求める
	/// GridUtils::GridToWorld を使用しているため、スケール変更時は GridUtils.h のみ変更すればよい
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

	/// <summary>
	/// 交差点かどうかを判定する
	/// </summary>
	bool IsIntersection(
		const GridCellComponent* cell,
		Direction movingDirection
	);

	/// <summary>
	/// 交差点通過時の処理（弾薬配置・回収可能化）
	/// </summary>
	void HandleIntersection(
		PlayerComponent* player,
		No::Registry& registry
	);

	/// <summary>
	/// 指定座標に弾薬アイテムが存在するかチェック
	/// </summary>
	bool HasAmmoAtPosition(
		No::Registry& registry,
		int gridX,
		int gridY
	);

	/// <summary>
	/// 弾薬アイテムを生成する
	/// </summary>
	void CreateAmmoItem(
		No::Registry& registry,
		int gridX,
		int gridY
	);

	/// <summary>
	/// 指定座標の弾薬アイテムを回収可能にする
	/// </summary>
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