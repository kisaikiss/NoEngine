#include "LevelDoorComponent.h"

REFLECT_STRUCT_BEGIN(LevelDoorComponent, "Application")
REFLECT_FIELD(requiredLevel),
REFLECT_FIELD(activationDistance),
REFLECT_FIELD(animationDuration),
REFLECT_FIELD(digitsOffset)
REFLECT_STRUCT_END(LevelDoorComponent)
