#pragma once

/// <summary>
/// ヘルスコンポーネント（Player・Enemy 共用）
/// 拡張性のため独立コンポーネントとして定義する。
///
///	使用例
///	Player ならmaxHp=3, currentHp=3 で AddComponent 後に設定
///	Enemy  → デフォルト値のまま AddComponent（maxHp=1, currentHp=1）HPといいつつ当たったら死ぬイメージ
/// </summary>
struct HealthComponent {
	
	/// HP
	int currentHp;		//現在の数値
	int maxHp;			//最大値

	/// デバッグ用無敵フラグ
	bool isInvincible;	// ダメージを受けない。ImGui でオン/オフを切り替えられる。
	
	/// ダメージ後の一時無敵タイマー
	float invincibilityTimer;	// ダメージを受けると INVINCIBILITY_DURATION がセットされ、0 になるまで再ダメージを受けない。敵との接触で毎フレームダメージを受けてしまう問題を防ぐ。
	static constexpr float INVINCIBILITY_DURATION = 1.0f;

	HealthComponent()
		: currentHp(1),
		maxHp(1),
		isInvincible(false),
		invincibilityTimer(0.0f) {
	}

	/// <summary>
	/// ダメージを受けられる状態かどうか
	/// </summary>
	bool CanTakeDamage() const {
		return !isInvincible && invincibilityTimer <= 0.0f;
	}

	/// <summary>
	/// ダメージを受ける。無敵状態なら何もしない。
	/// </summary>
	/// <returns>HPが0以下になった（死亡）場合 true</returns>
	bool TakeDamage(int amount = 1) {
		if (!CanTakeDamage()) return false;
		currentHp -= amount;
		invincibilityTimer = INVINCIBILITY_DURATION;
		return currentHp <= 0;
	}
};