#include "LightComponent.h"
#include "engine/Editor/ReflectionMacros.h"


REFLECT_STRUCT_BEGIN(NoEngine::Component::DirectionalLightComponent, "Light")
REFLECT_FIELD(color),
REFLECT_FIELD(direction),
REFLECT_FIELD(intensity)
REFLECT_STRUCT_END(NoEngine::Component::DirectionalLightComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::PointLightComponent, "Light")
REFLECT_FIELD(color),
REFLECT_FIELD(intensity),
REFLECT_FIELD(radius),
REFLECT_FIELD(decay)
REFLECT_STRUCT_END(NoEngine::Component::PointLightComponent)


REFLECT_STRUCT_BEGIN(NoEngine::Component::SpotLightComponent, "Light")
REFLECT_FIELD(color),
REFLECT_FIELD(intensity),
REFLECT_FIELD(distance),
REFLECT_FIELD(decay),
REFLECT_FIELD(cosAngle),
REFLECT_FIELD(cosFalloffStart)
REFLECT_STRUCT_END(NoEngine::Component::SpotLightComponent)