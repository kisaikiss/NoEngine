#include "FloatingComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::FloatingComponent, "Transform")
REFLECT_FIELD(velocity),
REFLECT_FIELD(amplitude)
REFLECT_STRUCT_END(NoEngine::Component::FloatingComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::Floating2DComponent, "Transform")
REFLECT_FIELD(velocity),
REFLECT_FIELD(amplitude)
REFLECT_STRUCT_END(NoEngine::Component::Floating2DComponent)