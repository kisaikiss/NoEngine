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

	float       bulletModelScale               = 0.25f;  // 弾モデルスケール（EnemyConfig::bulletModelScale からコピー）

	// 弾コライダー設定（EnemyConfig::bulletCollider から EnemySpawnSystem でコピーされる）
	float       bulletColliderRadiusMultiplier = 0.5f;
	No::Vector3 bulletColliderLocalOffset      = { 0.0f, 0.0f, 0.0f };
};
