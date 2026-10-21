#include "stdafx.h"
#include "StageIntroDisplayComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(StageIntroDisplayComponent, "Application")
REFLECT_FIELD(startDelay),
REFLECT_FIELD(fadeInDuration),
REFLECT_FIELD(holdDuration),
REFLECT_FIELD(fadeOutDuration)
REFLECT_STRUCT_END(StageIntroDisplayComponent)
