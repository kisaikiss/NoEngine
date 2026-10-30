#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
struct EventLink {
	std::string eventId;
	std::string actionId;
	ECS::Entity targetEntity = ECS::INVALID_ENTITY;
};

struct EventLinkComponent {
    std::vector<EventLink> links;
};
}
