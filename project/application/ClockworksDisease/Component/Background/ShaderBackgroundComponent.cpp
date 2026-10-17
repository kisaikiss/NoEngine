#include "ShaderBackgroundComponent.h"

REFLECT_STRUCT_BEGIN(ShaderBackgroundComponent, "AppEffect")
REFLECT_FIELD(shaderPath),
REFLECT_FIELD(isVisible),
REFLECT_FIELD(timeScale),
REFLECT_FIELD(color),
REFLECT_FIELD(param)
REFLECT_STRUCT_END(ShaderBackgroundComponent)