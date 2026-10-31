#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
// 旧来の直接イベント接続データ。既存シーンとの互換のためEventLinkSystemが引き続き処理する。
struct EventLink {
	std::string eventId;   // 発火条件のイベントID
	std::string actionId;  // 実行するAction ID
	ECS::Entity targetEntity = ECS::INVALID_ENTITY; // Actionに渡す対象。新規グラフではGraph側propertiesへ移行する。
};

struct EventLinkComponent {
	// 同一Entityに付いたイベントが来たときに評価する、旧形式の直接接続一覧。
    std::vector<EventLink> links;
};
}
