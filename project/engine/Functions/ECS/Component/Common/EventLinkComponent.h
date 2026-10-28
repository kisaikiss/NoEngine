#pragma once
#include "engine/Editor/Reflection/ReflectionMacros.h"

namespace NoEngine::Component {
struct EventLink {
    std::string eventName;
    ECS::Entity targetEntity;
};

struct EventLinkComponent {
    std::vector<EventLink> links;
};
}
