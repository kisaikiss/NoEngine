#pragma once
#include <string>

struct PlayerComponent {
	float speed = 200.f;					// 最大横移動速度 (px/s)
	float accelX = 1500.f;					// 横加速度 (px/s^2, 入力時)
	float frictionX = 900.f;				// 横摩擦 (px/s^2, 入力なし時の減速)
	float width = 36.f;						// 当たり判定幅 (px)
	float height = 36.f;					// 当たり判定高さ (px)
	float jumpSpeed = 400.f;				// ジャンプ初速 (px/s, GravityComponent が必要)

	// 壁ずり・壁ジャンプ
	float wallSlideSpeed = 80.f;			// 壁ずり中の最大落下速度 (px/s)
	float wallJumpSpeedX = 300.f;			// 壁ジャンプの横初速 (px/s)

	// フレーム状態（PlayerSystem が毎フレーム更新、読み取り専用）
	bool  isGrounded = false;				// 接地しているか
	bool  isTouchingWallLeft = false;		// 左に壁が接触しているか
	bool  isTouchingWallRight = false;		// 右に壁が接触しているか
	bool  isWallSliding = false;			// 壁ずり中か

	bool  isDead = false;					// true になったフレームで respawnX/Y へワープ
	float respawnX = 0.f;					// 現在有効なリスポーン地点 (px)
	float respawnY = 0.f;
	std::string respawnMap;					// 現在有効なリスポーン地点のマップ名
};
