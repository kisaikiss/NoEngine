#include "FollowCameraComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(FollowCameraComponent, "Application")
REFLECT_FIELD(maxDistance),
REFLECT_FIELD(distance),
REFLECT_FIELD(theta),
REFLECT_FIELD(phi),
REFLECT_FIELD(moveSpeed),
REFLECT_FIELD(minPhi),
REFLECT_FIELD(maxPhi),
REFLECT_FIELD(minFov),
REFLECT_FIELD(maxFov),
REFLECT_FIELD(playerPosOffset)
REFLECT_STRUCT_END(FollowCameraComponent)

