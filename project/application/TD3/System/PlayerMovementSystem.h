#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerComponent.h"
#include "../Component/GridCellComponent.h"

/// <summary>
/// プレイヤー移動システム
/// 入力処理・移動判定・ノード到達処理・モデル回転を担当する
/// </summary>
class PlayerMovementSystem : public No::ISystem {
public:
	/// <summary>
	/// 毎フレーム呼ばれるメイン更新処理
	/// プレイヤーの状態に応じた処理を振り分ける
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	// エッジ終点近傍での先行入力を有効にする閾値 [0.0 ~ 1.0]
	// この値以上の progress ではターゲットノードで有効な方向キーを押すだけで自動前進+方向転換が行われる
	static constexpr float NEAR_END_THRESHOLD = 0.75f;

	// ========== 状態別の入力・移動処理 ==========

	/// <summary>
	/// ノード上にいるときの入力処理
	/// 押されたキーと接続情報を照合し、移動を開始する
	/// </summary>
	void HandleNodeInput(
		PlayerComponent* player,
		No::Registry& registry
	);

	/// <summary>
	/// エッジ移動中の処理
	/// 現在方向キーの継続入力で前進し、ノードに到達したら OnReachNode を呼ぶ。
	/// エッジ終点 NEAR_END_THRESHOLD 以降は、ターゲットノードで有効な方向キーでも前進する。
	/// キーを離したらエッジ途中で停止する。
	/// </summary>
	void HandleEdgeMovement(
		PlayerComponent* player,
		float deltaTime,
		No::Registry& registry
	);

	/// <summary>
	/// エッジ途中で停止中のときの処理
	/// 来た方向のキーを押すと移動を再開する。
	/// エッジ終点 NEAR_END_THRESHOLD 以降は、ターゲットノードで有効な方向キーでも再開する。
	/// </summary>
	void HandleStoppedOnEdge(
		PlayerComponent* player,
		No::Registry& registry
	);

	/// <summary>
	/// 入力履歴の更新処理
	/// 進行方向以外の押下キーを記録し、離されたキーは削除する。
	/// 一定時間が経過したら履歴をクリアする。
	/// </summary>
	void UpdateRecentInputs(
		PlayerComponent* player,
		float deltaTime
	);

	/// <summary>
	/// ノード到達時の処理
	/// 現在座標をターゲットノードに更新し、次の移動方向を決定する。
	/// bufferedDirection → recentInputs → 現在入力 の優先順でチェックする。
	/// </summary>
	void OnReachNode(
		PlayerComponent* player,
		No::Registry& registry
	);

	/// <summary>
	/// 指定方向への移動を開始する
	/// ターゲットノード座標・移動状態・進行方向を設定する
	/// </summary>
	void StartMovement(
		PlayerComponent* player,
		Direction dir,
		No::Registry& registry
	);

	/// <summary>
	/// エッジ途中で停止する
	/// progressOnEdge はそのまま保持される
	/// </summary>
	void StopMovement(PlayerComponent* player);

	// ========== 先行入力 ==========

	/// <summary>
	/// エッジ終点近傍での有効方向キー取得
	/// ターゲットノードで移動できる方向のキーが押されているか調べ、
	/// 最初に見つかった有効方向を返す。なければ Direction::None を返す。
	/// 曲がり方向（現在の進行方向以外）を直進より優先して返す。
	/// </summary>
	Direction GetNearEndInputDirection(
		PlayerComponent* player,
		No::Registry& registry
	);

	// ========== 判定ヘルパー ==========

	/// <summary>
	/// 指定ノードから指定方向に移動できるか判定する
	/// 接続チェックと後退禁止ルールを考慮する（行き止まりの後退は例外許可）
	/// </summary>
	bool CanMoveInDirection(
		No::Registry& registry,
		int nodeX, int nodeY,
		Direction dir,
		Direction lastDir
	);

	/// <summary>
	/// 指定座標のグリッドセルを取得する
	/// 見つからない場合は nullptr を返す
	/// </summary>
	GridCellComponent* GetGridCell(
		No::Registry& registry,
		int x, int y
	);

	/// <summary>
	/// 指定方向への接続があるか返す
	/// </summary>
	bool HasConnection(
		const GridCellComponent* cell,
		Direction dir
	);

	/// <summary>
	/// 指定方向の反対方向を返す
	/// </summary>
	Direction GetOppositeDirection(Direction dir);

	/// <summary>
	/// 指定座標と方向から、次のノード座標を計算する
	/// </summary>
	void GetNextNodeCoords(
		int x, int y,
		Direction dir,
		int& outX, int& outY
	);

	// ========== Transform 更新 ==========

	/// <summary>
	/// プレイヤーのワールド座標を計算する
	/// OnNode のときはノード座標、エッジ上は線形補間で求める
	/// </summary>
	No::Vector3 CalculateWorldPosition(
		const PlayerComponent* player
	);

	/// <summary>
	/// プレイヤーの Transform（位置・回転）を更新する
	/// actualMovingDirection を使ってモデルを進行方向に向ける
	/// </summary>
	void UpdateTransform(
		PlayerComponent* player,
		No::TransformComponent* transform
	);

	/// <summary>
	/// 移動方向に対応する回転クオータニオンを返す
	/// モデルは identity 回転のとき Z+ を向いている前提（DirectX 左手座標系）
	///
	/// None  : Z- を向く（カメラ側・初期向き） = Y軸 +180°
	/// Up    : Y+ を向く                       = X軸  +90°
	/// Down  : Y- を向く                       = X軸  -90°
	/// Right : X+ を向く                       = Y軸  +90°
	/// Left  : X- を向く                       = Y軸  -90°
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

	// ========== デバッグ UI ==========
#ifdef USE_IMGUI
	void ShowPlayerDebugUI(PlayerComponent* player);
	const char* DirectionToString(Direction dir);
	const char* StateToString(PlayerState state);
#endif
};