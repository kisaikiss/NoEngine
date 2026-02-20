#include "Transform2DComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::Transform2DComponent)
REFLECT_FIELD(translate),
REFLECT_FIELD(rotation),
REFLECT_FIELD(scale)
REFLECT_STRUCT_END(NoEngine::Component::Transform2DComponent)