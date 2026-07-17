#pragma once
#include "engine/NoEngine.h"

enum class PlayerState {
	kWait,
	kWalk,
	kJump,
	kMultiJump,
	kHighJump,
	kFall
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


struct LevelComponent {
	uint32_t power = 0;
	uint32_t nowLevel = 1;
	uint32_t nextLevelUp = 30;
};