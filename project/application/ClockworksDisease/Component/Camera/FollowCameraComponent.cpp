#include "FollowCameraComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(FollowCameraComponent)
REFLECT_FIELD(distance),
REFLECT_FIELD(theta),
REFLECT_FIELD(phi),
REFLECT_FIELD(autoFollowDelay),
REFLECT_FIELD(maxAutoFollowAngularSpeed),
REFLECT_FIELD(autoFollowDeadzoneAngle)
REFLECT_STRUCT_END(FollowCameraComponent)

