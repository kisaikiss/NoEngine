#include "CameraComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::ActiveCameraTag)
REFLECT_STRUCT_END(NoEngine::Component::ActiveCameraTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::DebugCameraComponent)
REFLECT_ENUM_FIELD(moveType),
REFLECT_FIELD(unityMoveSpeed),
REFLECT_FIELD(unitySensitivity),
REFLECT_FIELD(distance),
REFLECT_FIELD(drawCenter),

REFLECT_FIELD(center)
REFLECT_STRUCT_END(NoEngine::Component::DebugCameraComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::CameraComponent)
REFLECT_FIELD(fov)
REFLECT_STRUCT_END(NoEngine::Component::CameraComponent)
