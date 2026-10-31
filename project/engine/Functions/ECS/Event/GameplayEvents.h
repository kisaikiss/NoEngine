#pragma once
#include "../Entity.h"

namespace NoEngine::Event {
namespace GameplayId {
// グラフやアプリケーションから参照する安定ID。表示名を変更しても保存データは変わらない。
inline constexpr char TriggerEnter[] = "engine.trigger.enter";
inline constexpr char TriggerExit[] = "engine.trigger.exit";
inline constexpr char MoverActivate[] = "engine.mover.activate";
inline constexpr char MoverStop[] = "engine.mover.stop";
inline constexpr char SoundPlay[] = "engine.sound.play";
inline constexpr char DebugLog[] = "engine.debug.log";
}

// EventLinkSystemへ渡す共通イベント。sourceは発行元、instigatorは接触者など発生原因のEntity。
struct GameplayEvent {
	ECS::Entity source = ECS::INVALID_ENTITY;	  // イベントを発行するEntity
	std::string eventId;						  // 登録済みイベントID
    ECS::Entity instigator = ECS::INVALID_ENTITY; // プレイヤーなど、きっかけになったEntity
};

// MoverActivate ActionからMoverSystemへ渡す、Mover再生開始専用の内部イベント。
struct ActivateMoverEvent {
	ECS::Entity targetEntity = ECS::INVALID_ENTITY;
};
}
