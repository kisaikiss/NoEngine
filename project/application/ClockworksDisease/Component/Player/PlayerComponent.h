#pragma once
#include "engine/NoEngine.h"

enum class PlayerState {
	kWait,					// 何もしていないとき
	kWalk,					// 歩いている時
	kJump,					// ジャンプ上昇中
	kAirDash,				// 空中ダッシュ中
	kHighJump,				// ハイジャンプ上昇中
	kFall					// 下降中
};

struct PlayerComponent {
	float moveSpeed = 10.f;
	float walkSpeed = 5.f;
	float dashStartInput = 0.5f;
	float doubleJumpSpeed = 4.f;
	float jumpSpeed = 16.f;
	float highJumpSpeed = 17.f;
	float airDashSpeed = 10.f;
	float deathHeight = -30.0f;
	No::Vector3 respawnPoint = No::Vector3::ZERO;
	// 空中で目標速度へ近づく速さ。小さいほど空中での慣性が強くなる。
	float airAcceleration = 20.f;
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

	float highJumpCostRate = 10.0f;
	float airDashStaminaCostRate = 2.0f;

	// コヨーテタイム
	float coyoteTime = 0.3f;
	float coyoteTimer = 0.0f;
};

struct PlayerAbilityDebugComponent {
	bool highJump = false;
	bool airDash = false;
	bool magicScaffold = false;
};

// PlayerJumpSystem / PlayerHorizontalMoveSystem / PlayerVerticalVelocitySystem の間で
// フレーム内だけ受け渡す一時データ。
// PlayerComponent（永続データ）とは意図的に分離している。
// 値は PlayerJumpSystem の冒頭で毎フレームリセットされる。
struct PlayerMoveTransientComponent {
	bool justJumped = false;   // このフレームでジャンプ入力を処理したか（接地/重力判定の分岐に使用）
	float slopeY = 0.f;        // 斜面投影による水平移動のy寄与分（PlayerHorizontalMoveSystemが算出）
	bool isAirDashing = false; // このフレーム、空中ダッシュ中か（移動速度/重力の分岐に使用）
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