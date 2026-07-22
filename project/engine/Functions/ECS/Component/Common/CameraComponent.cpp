#include "CameraComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::ActiveCameraTag, "Camera")
REFLECT_STRUCT_END(NoEngine::Component::ActiveCameraTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::ActiveCamera2DTag, "Camera")
REFLECT_STRUCT_END(NoEngine::Component::ActiveCamera2DTag)

REFLECT_STRUCT_BEGIN(NoEngine::Component::DebugCameraComponent, "Camera")
REFLECT_ENUM_FIELD(moveType),
REFLECT_FIELD(unityMoveSpeed),
REFLECT_FIELD(unitySensitivity),
REFLECT_FIELD(distance),
REFLECT_FIELD(drawCenter),

REFLECT_FIELD(center)
REFLECT_STRUCT_END(NoEngine::Component::DebugCameraComponent)



REFLECT_STRUCT_BEGIN(NoEngine::Component::DebugCamera2DComponent, "Camera")
REFLECT_STRUCT_END(NoEngine::Component::DebugCamera2DComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::CameraComponent, "Camera")
REFLECT_FIELD(fov)
REFLECT_STRUCT_END(NoEngine::Component::CameraComponent)

REFLECT_STRUCT_BEGIN(NoEngine::Component::Camera2DComponent, "Camera")
REFLECT_STRUCT_END(NoEngine::Component::Camera2DComponent)