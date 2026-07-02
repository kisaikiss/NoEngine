#include "PlayerComponent.h"


REFLECT_STRUCT_BEGIN(PlayerComponent)
REFLECT_FIELD(moveSpeed),
REFLECT_FIELD(jumpSpeed),
REFLECT_FIELD(doubleJumpSpeed),
REFLECT_FIELD(gravity),
REFLECT_FIELD(stamina),
REFLECT_FIELD(maxStamina),
REFLECT_FIELD(infinityJump),
REFLECT_FIELD(power)
REFLECT_STRUCT_END(PlayerComponent)
