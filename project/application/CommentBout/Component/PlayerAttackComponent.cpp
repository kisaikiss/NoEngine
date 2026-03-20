#include "PlayerAttackComponent.h"
#include "PauseStateComponent.h"
#include "PauseMenuConfigComponent.h"

REFLECT_STRUCT_BEGIN(PlayerAttackComponent)
REFLECT_FIELD(spawnOffset),
REFLECT_FIELD(attackSize),
REFLECT_FIELD(visibleTime),
REFLECT_FIELD(attackLayer)
REFLECT_STRUCT_END(PlayerAttackComponent)

REFLECT_STRUCT_BEGIN(PauseStateComponent)
REFLECT_FIELD(isPaused),
REFLECT_FIELD(justEnteredPause),
REFLECT_FIELD(justExitedPause),
REFLECT_FIELD(selectedIndex),
REFLECT_FIELD(itemCount)
REFLECT_STRUCT_END(PauseStateComponent)

REFLECT_STRUCT_BEGIN(PauseMenuConfigComponent)
REFLECT_FIELD(titlePosition),
REFLECT_FIELD(titleSize),
REFLECT_FIELD(itemBasePosition),
REFLECT_FIELD(itemSize),
REFLECT_FIELD(itemSpacing),
REFLECT_FIELD(cursorOffset),
REFLECT_FIELD(cursorSize),
REFLECT_FIELD(dimAlpha),
REFLECT_FIELD(openDuration),
REFLECT_FIELD(closeDuration),
REFLECT_FIELD(easeType),
REFLECT_FIELD(dimLayer),
REFLECT_FIELD(titleLayer),
REFLECT_FIELD(itemLayer),
REFLECT_FIELD(cursorLayer)
REFLECT_STRUCT_END(PauseMenuConfigComponent)
