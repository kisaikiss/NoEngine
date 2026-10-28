#pragma once
#include "../Entity.h"

namespace NoEngine::Event {
struct GameplayEvent {
	ECS::Entity source = ECS::INVALID_ENTITY;	  // イベントを発行するEntity
	std::string eventName;						  // イベント名
    ECS::Entity instigator = ECS::INVALID_ENTITY; // プレイヤーなど、きっかけになったEntity
};

struct ActivateMoverEvent {
	ECS::Entity targetEntity = ECS::INVALID_ENTITY;
};
}
