#pragma once
#include "engine/NoEngine.h"

enum class PlayerState {
	kWait,					// 何もしていないとき
	kWalk,					// 歩いている時
	kJump,					// ジャンプ上昇中
	kAirDash,				// 空中ダッシュ中
	kFall,					// 下降中
	kSleep,					// 寝っ転がる
};

// ハイジャンプは独立した行動状態ではなく、JumpStateの種類として扱う。
enum class PlayerJumpKind {
	kNormal,
	kHighJump,
};

struct PlayerComponent {
	float moveSpeed = 10.f;
	float walkSpeed = 5.f;
	float dashStartInput = 0.5f;
	float doubleJumpSpeed = 4.f;
	float jumpSpeed = 16.f;
	float highJumpSpeed = 17.f;
	float airDashSpeed = 10.f;
	// 回転攻撃のワールド座標での判定半径。
	float spinAttackRadius = 1.5f;
	// HurricaneKickの再生速度倍率。1.0で元の速度。
	float spinAttackAnimationSpeed = 1.0f;
	float deathHeight = -30.0f;
	No::Vector3 respawnPoint = No::Vector3::ZERO;
	// 空中で目標速度へ近づく速さ。小さいほど空中での慣性が強くなる。
	float airAcceleration = 20.f;
	// 接地中に最高速（現在の歩行/ダッシュ速度）へ到達するまでの時間。0以下なら即時に速度を合わせる。
	float accelerationTime = 0.35f;
	float gravity = -9.8f;
	float yVelocity = 0.f;
	float maxFallSpeed = -25.f;
	float stamina = 0.0f;
	float maxStamina = 0.0f;
	float staminaRecoveryRate = 1.0f;   // 秒あたりのスタミナ回復量
	float staminaUpPerLevel = 1.0f;     // レベルアップ時のスタミナ最大値上昇量
 	No::Vector3 groundNormal = No::Vector3::UP;
	// この値未満の上向き法線は壁・角への接触として扱い、接地しない。
	float minGroundNormalY = 0.75f;
	bool infinityJump = false;
	bool infinityStamina = false;
	bool canCreateScaffold = true;
	PlayerState state = PlayerState::kWait;
	PlayerJumpKind jumpKind = PlayerJumpKind::kNormal;

	float highJumpCostRate = 10.0f;
	float airDashStaminaCostRate = 2.0f;

	// コヨーテタイム
	float coyoteTime = 0.3f;
	float coyoteTimer = 0.0f;
	// 通常ジャンプを着地前に受け付ける時間（秒）。0以下なら先行入力を無効にする。
	float jumpBufferTime = 0.15f;
};

struct PlayerAbilityDebugComponent {
	bool highJump = false;
	bool airDash = false;
	bool magicScaffold = false;
};

// PlayerInputSystemが物理入力を1フレーム分だけ収集し、プレイヤー処理へ渡す。
// ゲームプレイSystemからInputConfigを直接参照しないための境界。
struct PlayerInputComponent {
	No::Vector3 moveAxis = No::Vector3::ZERO;
	bool jumpTriggered = false;
	bool jumpReleased = false;
	bool highJumpTriggered = false;
	bool highJumpPressed = false;
	bool highJumpReleased = false;
	bool airDashPressed = false;
	bool airDashReleased = false;
	bool createScaffoldTriggered = false;
	bool attackTriggered = false;
};

enum class PlayerActionType {
	kNone,
	kAirDash,
	kSpinAttack,
};

// 実行中アクションの共通状態。移動・物理Systemはこの状態を参照して効果を適用する。
struct PlayerActionComponent {
	PlayerActionType activeAction = PlayerActionType::kNone;
	float elapsedTime = 0.0f;
	float animationSpeedBeforeSpinAttack = 1.0f;
};

// PlayerJumpSystem / PlayerHorizontalMoveSystem / PlayerVerticalVelocitySystem が共有する
// 移動処理の実行時データ。シーンへの保存対象にはしない。
// PlayerComponent（永続データ）とは意図的に分離している。
// justJumpedとslopeYはPlayerJumpSystemの冒頭で毎フレームリセットされる。
struct PlayerMoveTransientComponent {
	bool justJumped = false;   // このフレームでジャンプ入力を処理したか（接地/重力判定の分岐に使用）
	float slopeY = 0.f;        // 斜面投影による水平移動のy寄与分（PlayerHorizontalMoveSystemが算出）
	float jumpBufferTimer = 0.f; // 未消費の通常ジャンプ入力の残り保持時間
	bool bufferedJumpReleased = false; // 着地前に離した入力でも小ジャンプにする
};


enum class PlayerAbility {
	kNone,
	kMultiJump,
	kHighJump,
	kAirDash,
	kMagicScaffold,
};

struct LevelUpReward {
	uint32_t level = 0;
	PlayerAbility ability = PlayerAbility::kNone;
};

struct LevelComponent {
	uint32_t power = 0;
	uint32_t nowLevel = 1;
	uint32_t nextLevelUp = 30;
	// レベルごとの必要経験値テーブル。index[0] = Lv1→2, index[1] = Lv2→3 ...
	// テーブルの長さを超えたレベルでは最後の要素を使い続ける。
	std::vector<uint32_t> levelUpRequirements;

	std::vector<LevelUpReward> rewards;
};
