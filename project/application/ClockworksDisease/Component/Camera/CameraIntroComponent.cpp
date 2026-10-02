#include "CameraIntroComponent.h"

REFLECT_STRUCT_BEGIN(CameraLockTag, "ApplicationTag")
REFLECT_STRUCT_END(CameraLockTag)

REFLECT_STRUCT_BEGIN(CameraIntroComponent, "Application")
REFLECT_ENUM_FIELD(phase),
REFLECT_FIELD(fadeOutDuration),
REFLECT_FIELD(blackOutDuration),
REFLECT_FIELD(fadeInDuration),
REFLECT_FIELD(playInEditor)
REFLECT_STRUCT_END(CameraIntroComponent)