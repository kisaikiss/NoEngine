#pragma once

/// <summary>
/// 戦闘対象の共通HP状態。
/// Player/Enemyを問わず同じ解決系で扱えるようにするための基盤コンポーネント。
/// </summary>
struct HealthComponent {
	int hp = 10;
	int maxHp = 10;
	bool isDead = false;
	int lastDamageTaken = 0;
};
