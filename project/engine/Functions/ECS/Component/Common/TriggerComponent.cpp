#include "TriggerComponent.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::TriggerComponent, "Gameplay")
REFLECT_FIELD(oneShot),
REFLECT_FIELD(activated),
REFLECT_FIELD(wasTouching)
REFLECT_STRUCT_END(NoEngine::Component::TriggerComponent)
