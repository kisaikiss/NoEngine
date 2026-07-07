#include "SpriteComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::SpriteComponent)
REFLECT_FIELD(isVisible),
REFLECT_FIELD(pivot),
REFLECT_FIELD(flipX),
REFLECT_FIELD(flipY),
REFLECT_FIELD(layer),
REFLECT_FIELD(orderInLayer),
REFLECT_FIELD(color),
REFLECT_FIELD(uv),
REFLECT_FIELD(textureName),
REFLECT_FIELD(nonRenderAngle),
REFLECT_FIELD(useMask),
REFLECT_FIELD(fill),
REFLECT_FIELD(maskTextureName)
REFLECT_STRUCT_END(NoEngine::Component::SpriteComponent)
