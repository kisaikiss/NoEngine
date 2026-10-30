#pragma once
#include "../Entity.h"

namespace NoEngine::Event {
namespace GameplayId {
inline constexpr char TriggerEnter[] = "engine.trigger.enter";
inline constexpr char MoverActivate[] = "engine.mover.activate";
}

struct GameplayEvent {
	ECS::Entity source = ECS::INVALID_ENTITY;	  // イベントを発行するEntity
	std::string eventId;						  // 登録済みイベントID
    ECS::Entity instigator = ECS::INVALID_ENTITY; // プレイヤーなど、きっかけになったEntity
};

struct ActivateMoverEvent {
	ECS::Entity targetEntity = ECS::INVALID_ENTITY;
};
}
