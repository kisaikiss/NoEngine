#include "EventLinkComponent.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::EventLink, "Other")
REFLECT_FIELD(eventName),
REFLECT_FIELD(targetEntity)
REFLECT_STRUCT_END(NoEngine::Component::EventLink)


REFLECT_STRUCT_BEGIN(NoEngine::Component::EventLinkComponent, "Gameplay")
REFLECT_STRUCT_ARRAY_FIELD(links)
REFLECT_STRUCT_END(NoEngine::Component::EventLinkComponent)

