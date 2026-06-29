#include "CameraComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::ActiveCameraTag)
REFLECT_STRUCT_END(NoEngine::Component::ActiveCameraTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::DebugCameraComponent)
REFLECT_FIELD(center),
REFLECT_FIELD(mousePositionX),
REFLECT_FIELD(mousePositionY),
REFLECT_FIELD(mouseWheelY),
REFLECT_FIELD(distance),
REFLECT_FIELD(theta),
REFLECT_FIELD(phi),
REFLECT_FIELD(drawCenter)
REFLECT_STRUCT_END(NoEngine::Component::DebugCameraComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::CameraComponent)
REFLECT_FIELD(fov)
REFLECT_STRUCT_END(NoEngine::Component::CameraComponent)
