#pragma once

/// <summary>
/// 敵弾発射設定。
/// EnemyShootSystem が interval/cooldown を参照して弾生成する
/// </summary>
struct EnemyShooterComponent {
	float shootInterval = 1.0f;
	float shootCooldown = 0.0f;
	float bulletSpeed = 8.0f;
	int bulletDamage = 1;
	float targetDepthFromCamera = 1.0f;
	float bulletLifetime = 4.0f;
	float shootDistanceMax = 25.0f;
};
