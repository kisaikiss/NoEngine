#include "UserInterfaceComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(StaminaGaugeComponent, "Application")
REFLECT_FIELD(disappearanceTime)
REFLECT_STRUCT_END(StaminaGaugeComponent)


REFLECT_STRUCT_BEGIN(LevelUpTextComponent, "Application")
REFLECT_FIELD(isClose),
REFLECT_FIELD(closeTime)
REFLECT_STRUCT_END(LevelUpTextComponent)


REFLECT_STRUCT_BEGIN(StaminaGaugeParentTag, "ApplicationTag")
REFLECT_STRUCT_END(StaminaGaugeParentTag)


REFLECT_STRUCT_BEGIN(CanMagicUITag, "ApplicationTag")
REFLECT_FIELD(isBackground)
REFLECT_STRUCT_END(CanMagicUITag)
