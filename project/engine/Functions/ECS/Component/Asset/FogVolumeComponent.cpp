#include "FogVolumeComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::FogVolumeComponent, "Effect")
REFLECT_FIELD(enabled),
REFLECT_FIELD(density),
REFLECT_FIELD(edgeFade),
REFLECT_FIELD(noiseFrequency),
REFLECT_FIELD(noiseStrength),
REFLECT_FIELD(windVelocity)
REFLECT_STRUCT_END(NoEngine::Component::FogVolumeComponent)
