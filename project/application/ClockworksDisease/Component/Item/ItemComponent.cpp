#include "ItemComponent.h"
#include "engine/NoEngine.h"


REFLECT_STRUCT_BEGIN(PowerItemComponent, "Application")
REFLECT_STRUCT_END(PowerItemComponent)


REFLECT_STRUCT_BEGIN(BigPowerItemComponent, "Application")
REFLECT_FIELD(grantPower)
REFLECT_STRUCT_END(BigPowerItemComponent)

REFLECT_STRUCT_BEGIN(SavePointComponent, "Application")
REFLECT_FIELD(rotateTime),
REFLECT_FIELD(colorMagnification)
REFLECT_STRUCT_END(SavePointComponent)