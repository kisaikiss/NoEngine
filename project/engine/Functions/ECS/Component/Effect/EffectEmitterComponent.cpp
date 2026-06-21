#include "stdafx.h"
#include "EffectEmitterComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::EffectEmitTag)
REFLECT_STRUCT_END(NoEngine::Component::EffectEmitTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::EffectEmitModeCircleTag)
REFLECT_FIELD(useXYPlane)
REFLECT_STRUCT_END(NoEngine::Component::EffectEmitModeCircleTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::EffectEmitModeRingTag)
REFLECT_STRUCT_END(NoEngine::Component::EffectEmitModeRingTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::EffectEmitModeSphereTag)
REFLECT_STRUCT_END(NoEngine::Component::EffectEmitModeSphereTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::EffectEmitterComponent)
REFLECT_FIELD(count),
REFLECT_FIELD(maxSpeed),
REFLECT_FIELD(minSpeed),
REFLECT_FIELD(maxScale),
REFLECT_FIELD(minScale),
REFLECT_FIELD(maxLifeTime),
REFLECT_FIELD(minLifeTime),
REFLECT_FIELD(color),
REFLECT_FIELD(maxRotate),
REFLECT_FIELD(minRotate),
REFLECT_FIELD(emitRange),
REFLECT_FIELD(textureName)
REFLECT_STRUCT_END(NoEngine::Component::EffectEmitterComponent)
