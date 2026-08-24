#include "TransformRoutineComponent2D.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::TransformKeyframe2D)
REFLECT_FIELD(translate),
REFLECT_FIELD(rotation),
REFLECT_FIELD(scale),
REFLECT_FIELD(duration),
REFLECT_ENUM_FIELD(easing)
REFLECT_STRUCT_END(NoEngine::TransformKeyframe2D)

REFLECT_STRUCT_BEGIN(NoEngine::Component::TransformRoutineComponent2D, "Transform")
REFLECT_STRUCT_ARRAY_FIELD(keyframes),
REFLECT_ENUM_FIELD(interpolation),
REFLECT_FIELD(loop),
REFLECT_FIELD(playing),
REFLECT_FIELD(playbackSpeed)
REFLECT_STRUCT_END(NoEngine::Component::TransformRoutineComponent2D)