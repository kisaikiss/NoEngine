#pragma once
struct StaminaGaugeComponent {
	float preStamina = 0.0f;
	float timer = 0.0f;
	float alphaT = 0.0f;
	float disappearanceTime = 3.0f; // 非表示までの時間(秒)
};


struct LevelUpTextComponent {
	float t = 0.0f;
	bool isClose = false;
	float closeTimer = 0.f;
	float closeTime = 3.f;
};

struct StaminaGaugeParentTag{};

// レベルアップした瞬間にUIへ付けるタグ
struct LevelUpFrameTag{};

struct LevelUpTextParentTag {};

struct CanMagicUITag { bool isBackground = false; };