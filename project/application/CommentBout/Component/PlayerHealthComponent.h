#pragma once

/// <summary>
/// 旧プレイヤーHP表現。
/// 現在は HealthComponent / InvincibleComponent が主系で、
/// この構造体は互換表示・段階移行のために保持する。
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
