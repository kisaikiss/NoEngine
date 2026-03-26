#include "OptionStateComponent.h"

REFLECT_STRUCT_BEGIN(OptionStateComponent)
REFLECT_FIELD(isOpen),
REFLECT_FIELD(phase),
REFLECT_FIELD(phaseTime),
REFLECT_FIELD(phaseDuration),
REFLECT_FIELD(selectedIndex),
REFLECT_FIELD(itemCount),
REFLECT_FIELD(isEditing),
REFLECT_FIELD(isConfirmAnimating),
REFLECT_FIELD(confirmIndex),
REFLECT_FIELD(confirmAnimTime),
REFLECT_FIELD(requestedAction),
REFLECT_FIELD(masterVolume),
REFLECT_FIELD(bgmVolume),
REFLECT_FIELD(seVolume),
REFLECT_FIELD(vibrationEnabled)
REFLECT_STRUCT_END(OptionStateComponent)
