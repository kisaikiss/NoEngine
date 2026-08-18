#include "CameraIntroComponent.h"

REFLECT_STRUCT_BEGIN(CameraIntroLockTag, "ApplicationTag")
REFLECT_STRUCT_END(CameraIntroLockTag)

REFLECT_STRUCT_BEGIN(CameraIntroComponent, "Application")
REFLECT_ENUM_FIELD(phase),
REFLECT_FIELD(fadeOutDuration),
REFLECT_FIELD(fadeInDuration),
REFLECT_FIELD(playInEditor)
REFLECT_STRUCT_END(CameraIntroComponent)