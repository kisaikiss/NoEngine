#include "MaterialComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::MaterialComponent)
REFLECT_FIELD(color),
REFLECT_FIELD(drawOutline),
REFLECT_FIELD(shininess),
REFLECT_FIELD(enviromentCoefficient),
REFLECT_FIELD(psoName)
REFLECT_STRUCT_END(NoEngine::Component::MaterialComponent)
