#include "LightComponent.h"
#include "engine/Editor/ReflectionMacros.h"


REFLECT_STRUCT_BEGIN(NoEngine::Component::DirectionalLightComponent)
REFLECT_FIELD(color),
REFLECT_FIELD(direction),
REFLECT_FIELD(intensity)
REFLECT_STRUCT_END(NoEngine::Component::DirectionalLightComponent)
