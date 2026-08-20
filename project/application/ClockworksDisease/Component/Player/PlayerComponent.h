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
	float doubleJumpSpeed = 4.f;
	float jumpSpeed = 16.f;
	float highJumpSpeed = 17.f;
	float airDashSpeed = 10.f;
	float gravity = -9.8f;
	float yVelocity = 0.f;
	float stamina = 0.0f;
	float maxStamina = 0.0f;
 	No::Vector3 groundNormal = No::Vector3::UP;
	bool infinityJump = false;
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

	std::vector<LevelUpReward> rewards;
};