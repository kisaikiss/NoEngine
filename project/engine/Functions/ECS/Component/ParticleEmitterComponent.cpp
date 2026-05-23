#include "ParticleEmitterComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::ParticleEmitterComponent)
REFLECT_FIELD(active),
REFLECT_FIELD(maxCount),
REFLECT_FIELD(count),
REFLECT_FIELD(frequency),
REFLECT_FIELD(maxSpeed),
REFLECT_FIELD(minSpeed),
REFLECT_FIELD(maxLifeTime),
REFLECT_FIELD(minLifeTime)
REFLECT_STRUCT_END(NoEngine::Component::ParticleEmitterComponent)