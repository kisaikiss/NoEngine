#include "MeshComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::MeshComponent, "Asset")
REFLECT_FIELD(isVisible),
REFLECT_FIELD(meshName)
REFLECT_STRUCT_END(NoEngine::Component::MeshComponent)
