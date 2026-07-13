#pragma once
struct StaminaGaugeComponent {
	float preStamina = 0.0f;
	float timer = 0.0f;
	float alphaT = 0.0f;
	float disappearanceTime = 3.0f; // 非表示までの時間(秒)
};

struct StaminaGaugeParentTag{};