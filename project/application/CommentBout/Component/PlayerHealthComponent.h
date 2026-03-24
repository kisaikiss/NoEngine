#pragma once

struct PlayerHealthComponent {
	int hp = 10;
	int maxHp = 10;
	float invincibleTime = 0.0f;
	float invincibleDuration = 0.35f;
	bool isDead = false;
	bool deathHandled = false;
	int lastDamageTaken = 0;
};
