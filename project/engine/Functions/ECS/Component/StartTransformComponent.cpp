#include "StartTransformComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::StartTransformComponent)
REFLECT_FIELD(translate),
REFLECT_FIELD(rotation),
REFLECT_FIELD(scale)
REFLECT_STRUCT_END(NoEngine::Component::StartTransformComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::StartTransform2DComponent)
REFLECT_FIELD(translate),
REFLECT_FIELD(rotation),
REFLECT_FIELD(scale)
REFLECT_STRUCT_END(NoEngine::Component::StartTransform2DComponent)