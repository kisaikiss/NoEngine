#include "DissolveComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::DissolveComponent)
REFLECT_FIELD(threshold),
REFLECT_FIELD(maskTextureName)
REFLECT_STRUCT_END(NoEngine::Component::DissolveComponent)