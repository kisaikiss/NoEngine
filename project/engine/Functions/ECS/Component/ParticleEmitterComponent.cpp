#include "ParticleEmitterComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::ParticleEmitterComponent)
REFLECT_FIELD(active),
REFLECT_FIELD(maxCount),
REFLECT_FIELD(count),
REFLECT_FIELD(frequency),
REFLECT_FIELD(maxSpeed),
REFLECT_FIELD(minSpeed),
REFLECT_FIELD(maxScale),
REFLECT_FIELD(minScale),
REFLECT_FIELD(maxLifeTime),
REFLECT_FIELD(minLifeTime),
REFLECT_FIELD(emitRange),
REFLECT_FIELD(localPosition),
REFLECT_FIELD(color),
REFLECT_FIELD(textureName)
REFLECT_STRUCT_END(NoEngine::Component::ParticleEmitterComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::ParticleEmitterSphereComponent)
REFLECT_FIELD(radius),
REFLECT_FIELD(count),
REFLECT_FIELD(frequency),
REFLECT_FIELD(frequencyTime)
REFLECT_STRUCT_END(NoEngine::Component::ParticleEmitterSphereComponent)