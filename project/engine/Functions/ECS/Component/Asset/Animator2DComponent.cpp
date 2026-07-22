#include "Animator2DComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::Animator2DComponent, "Asset")
REFLECT_FIELD(currentAnimation),
REFLECT_FIELD(animeFrameWidth),
REFLECT_FIELD(animeFrameHeight),
REFLECT_FIELD(frameByFrameTime),
REFLECT_FIELD(framesNum)
REFLECT_STRUCT_END(NoEngine::Component::Animator2DComponent)