#include "stdafx.h"
#include "EffectEmitterComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::EffectEmitTag, "Effect")
REFLECT_STRUCT_END(NoEngine::Component::EffectEmitTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::EffectEmitterComponent, "Effect")
REFLECT_FIELD(startDelay),
REFLECT_FIELD(burstCount),
REFLECT_FIELD(burstInterval),
REFLECT_FIELD(count),
REFLECT_ENUM_FIELD(spawnShape),
REFLECT_FIELD(spawnOffset),
REFLECT_FIELD(emitRange),
REFLECT_FIELD(spawnRadius),
REFLECT_ENUM_FIELD(velocityDistribution),
REFLECT_FIELD(maxSpeed),
REFLECT_FIELD(minSpeed),
REFLECT_FIELD(maxScale),
REFLECT_FIELD(minScale),
REFLECT_FIELD(maxLifeTime),
REFLECT_FIELD(minLifeTime),
REFLECT_FIELD(color),
REFLECT_FIELD(maxRotate),
REFLECT_FIELD(minRotate),
REFLECT_ENUM_FIELD(shape),
REFLECT_ASSET_FIELD(textureName, Texture)
REFLECT_STRUCT_END(NoEngine::Component::EffectEmitterComponent)
