#include "MaterialComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::MaterialComponent)
REFLECT_ENUM_FIELD(blendMode),
REFLECT_ENUM_FIELD(renderMode),
REFLECT_FIELD(color),
REFLECT_FIELD(drawOutline),
REFLECT_FIELD(shininess),
REFLECT_FIELD(enviromentCoefficient),
REFLECT_FIELD(uvPosition),
REFLECT_FIELD(uvRotate),
REFLECT_FIELD(uvScale),
REFLECT_FIELD(emissiveIntensity),
REFLECT_FIELD(rimPower),
REFLECT_FIELD(noiseScrollSpeed),
REFLECT_FIELD(edgeColor),
REFLECT_FIELD(noiseTextureName)
REFLECT_STRUCT_END(NoEngine::Component::MaterialComponent)
