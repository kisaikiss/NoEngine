#include "PlayerComponent.h"


REFLECT_STRUCT_BEGIN(PlayerComponent)
REFLECT_FIELD(moveSpeed),
REFLECT_FIELD(jumpSpeed),
REFLECT_FIELD(highJumpSpeed),
REFLECT_FIELD(doubleJumpSpeed),
REFLECT_FIELD(airDashSpeed),
REFLECT_FIELD(gravity),
REFLECT_FIELD(stamina),
REFLECT_FIELD(maxStamina),
REFLECT_FIELD(infinityJump),
REFLECT_ENUM_FIELD(state)
REFLECT_STRUCT_END(PlayerComponent)

REFLECT_STRUCT_BEGIN(PlayerMoveTransientComponent)
REFLECT_STRUCT_END(PlayerMoveTransientComponent)

REFLECT_STRUCT_BEGIN(LevelComponent)
REFLECT_FIELD(nowLevel),
REFLECT_FIELD(nextLevelUp),
REFLECT_FIELD(power)
REFLECT_STRUCT_END(LevelComponent)
