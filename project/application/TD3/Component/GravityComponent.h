#pragma once

/// <summary>
/// 重力コンポーネント。
/// 垂直速度と重力加速度を保持する。
/// PlayerSystem など各システムがこの値を読み書きして物理挙動を実現する。
/// </summary>
struct GravityComponent {
	float gravity      = 980.f;			// 重力加速度 (px/s^2, 下方向が正)
	float velocityY    = 0.f;			// 現在の垂直速度 (px/s, 下方向が正)
	float velocityX    = 0.f;			// 物理横速度 (px/s, 壁ジャンプ時に使用)
	float maxFallSpeed = 1200.f;		// 最大落下速度 (px/s)
};
