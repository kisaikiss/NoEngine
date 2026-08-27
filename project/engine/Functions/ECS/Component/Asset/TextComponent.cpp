#include "TextComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::TextComponent, "Asset")
REFLECT_FIELD(text),
REFLECT_FIELD(scale),
REFLECT_FIELD(letterSpacing),
REFLECT_FIELD(layer),
REFLECT_FIELD(orderInLayer),
REFLECT_FIELD(isVisible),
REFLECT_ENUM_FIELD(hAlign),
REFLECT_ENUM_FIELD(vAlign),
REFLECT_ENUM_FIELD(space),
REFLECT_FIELD(anchor),
REFLECT_FIELD(color)
REFLECT_STRUCT_END(NoEngine::Component::TextComponent)