#pragma once
#include "engine/NoEngine.h"

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

struct StaminaGaugeParentTag {};

// レベルアップした瞬間にUIへ付けるタグ
struct LevelUpFrameTag {};

struct LevelUpTextParentTag {};

struct CanMagicUITag { bool isBackground = false; };

// LevelUpTextComponentと同じEntityに付与し、表示待ちのヒントテクスチャ名を貯めておくキュー。
// ヒント表示中(LevelUpFrameTagが付いている間)に新たなヒントが発生した場合はここに積まれ、
// 表示中のヒントが完全に右へはけたタイミングで先頭から取り出して次のヒントを表示する。
struct LevelUpHintQueueComponent {
	std::deque<std::string> pendingTextureNames;
};

// レベルアップヒントのアイコンEntityを1つ生成する。
// 生成したEntityにはLevelUpTextParentTagが付き、位置はLevelUpTextSystemがヒント枠に追従させ、
// 表示が終わった際の破棄もLevelUpTextSystemが行う。
No::Entity CreateLevelUpHintEntity(No::Registry& registry, const std::string& textureName);