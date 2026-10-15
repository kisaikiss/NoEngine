#include "RotationalVelocityComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::RotationalVelocityComponent, "Transform")
REFLECT_FIELD(linear)
REFLECT_STRUCT_END(NoEngine::Component::RotationalVelocityComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::RotationalVelocity2DComponent, "Transform")
REFLECT_FIELD(linear)
REFLECT_STRUCT_END(NoEngine::Component::RotationalVelocity2DComponent)