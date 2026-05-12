#include "VelocityComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::VelocityComponent)
REFLECT_FIELD(linear)
REFLECT_STRUCT_END(NoEngine::Component::VelocityComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::Velocity2DComponent)
REFLECT_FIELD(linear)
REFLECT_STRUCT_END(NoEngine::Component::Velocity2DComponent)