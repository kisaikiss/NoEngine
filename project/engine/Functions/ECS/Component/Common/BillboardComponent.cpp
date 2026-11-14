#include "BillboardComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::BillboardComponent, "Rendering")
REFLECT_FIELD(enabled),
REFLECT_ENUM_FIELD(facing),
REFLECT_ENUM_FIELD(constraint),
REFLECT_ENUM_FIELD(cameraSource),
REFLECT_FIELD(axis),
REFLECT_FIELD(rotationOffset)
REFLECT_STRUCT_END(NoEngine::Component::BillboardComponent)
