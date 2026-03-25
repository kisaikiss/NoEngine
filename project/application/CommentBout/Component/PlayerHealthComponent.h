#pragma once

/// <summary>
/// 旧プレイヤーHP表現（互換用）。
/// 主系は HealthComponent / InvincibleComponent で、本コンポーネントは段階移行中の同期先として保持する
/// </summary>
struct PlayerHealthComponent {
	int hp = 10;
	int maxHp = 10;
	float invincibleTime = 0.0f;
	float invincibleDuration = 0.35f;
	bool isDead = false;
	bool deathHandled = false;
	int lastDamageTaken = 0;
};
