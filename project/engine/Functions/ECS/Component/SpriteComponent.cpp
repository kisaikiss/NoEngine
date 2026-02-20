#include "SpriteComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::SpriteComponent)
REFLECT_FIELD(pivot),
REFLECT_FIELD(flipX),
REFLECT_FIELD(flipY),
REFLECT_FIELD(color)
REFLECT_STRUCT_END(NoEngine::Component::SpriteComponent)
