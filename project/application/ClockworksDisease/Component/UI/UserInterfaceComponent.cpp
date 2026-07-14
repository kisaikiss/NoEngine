#include "UserInterfaceComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(StaminaGaugeComponent)
REFLECT_FIELD(disappearanceTime)
REFLECT_STRUCT_END(StaminaGaugeComponent)


REFLECT_STRUCT_BEGIN(LevelUpTextComponent)
REFLECT_FIELD(isClose),
REFLECT_FIELD(closeTime)
REFLECT_STRUCT_END(LevelUpTextComponent)


REFLECT_STRUCT_BEGIN(StaminaGaugeParentTag)
REFLECT_STRUCT_END(StaminaGaugeParentTag)
