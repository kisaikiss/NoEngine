#include "FollowCameraComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(FollowCameraComponent, "Application")
REFLECT_FIELD(distance),
REFLECT_FIELD(theta),
REFLECT_FIELD(phi)
REFLECT_STRUCT_END(FollowCameraComponent)

