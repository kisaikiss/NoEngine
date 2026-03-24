#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// ダメージ適用要求。
/// 直接HPを減らさず、要求を一度キュー化して専用Systemで適用するために使用する。
/// </summary>
struct DamageRequestComponent {
	No::Entity target = No::nullEntity;
	No::Entity source = No::nullEntity;
	int amount = 1;
	bool ignoreInvincible = false;
};
