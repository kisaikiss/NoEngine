#pragma once

/// <summary>
/// 一定間隔で弾を発射する敵の初期設定。
/// </summary>
struct EnemyShooterComponent {
	float shootInterval = 1.0f;			//
	float shootCooldown = 0.0f;			//
	float bulletSpeed = 8.0f;			//
	int bulletDamage = 1;				//
	float targetDepthFromCamera = 1.0f;	//
	float bulletLifetime = 4.0f;		//
};
