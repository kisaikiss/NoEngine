#include "PauseStateComponent.h"

REFLECT_STRUCT_BEGIN(PauseStateComponent)
REFLECT_FIELD(isPaused),
REFLECT_FIELD(justEnteredPause),
REFLECT_FIELD(justExitedPause),
REFLECT_FIELD(selectedIndex),
REFLECT_FIELD(itemCount),
REFLECT_FIELD(phase),
REFLECT_FIELD(phaseTime),
REFLECT_FIELD(phaseDuration),
REFLECT_FIELD(requestedAction),
REFLECT_FIELD(isConfirmAnimating),
REFLECT_FIELD(confirmIndex),
REFLECT_FIELD(confirmAnimTime)
REFLECT_STRUCT_END(PauseStateComponent)