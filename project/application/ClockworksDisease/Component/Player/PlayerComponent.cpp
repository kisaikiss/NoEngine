#include "PlayerComponent.h"


REFLECT_STRUCT_BEGIN(PlayerComponent, "Application")
REFLECT_FIELD(moveSpeed),
REFLECT_FIELD(jumpSpeed),
REFLECT_FIELD(highJumpSpeed),
REFLECT_FIELD(doubleJumpSpeed),
REFLECT_FIELD(airDashSpeed),
REFLECT_FIELD(gravity),
REFLECT_FIELD(stamina),
REFLECT_FIELD(maxStamina),
REFLECT_FIELD(coyoteTime),
REFLECT_FIELD(coyoteTimer),
REFLECT_FIELD(infinityJump),
REFLECT_ENUM_FIELD(state)
REFLECT_STRUCT_END(PlayerComponent)

REFLECT_STRUCT_BEGIN(PlayerMoveTransientComponent, "ApplicationTag")
REFLECT_STRUCT_END(PlayerMoveTransientComponent)

REFLECT_STRUCT_BEGIN(LevelUpReward, "ApplicationTag")
REFLECT_FIELD(level),
REFLECT_ENUM_FIELD(ability)
REFLECT_STRUCT_END(LevelUpReward)

REFLECT_STRUCT_BEGIN(LevelComponent, "Application")
REFLECT_FIELD(nowLevel),
REFLECT_FIELD(nextLevelUp),
REFLECT_FIELD(power),
REFLECT_STRUCT_ARRAY_FIELD(rewards)
REFLECT_STRUCT_END(LevelComponent)
