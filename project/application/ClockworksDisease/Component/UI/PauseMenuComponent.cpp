#include "PauseMenuComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(PauseMenuComponent, "Application")
REFLECT_ENUM_FIELD(state),
REFLECT_FIELD(mainSelectedIndex),
REFLECT_FIELD(optionSelectedIndex)
REFLECT_STRUCT_END(PauseMenuComponent)

REFLECT_STRUCT_BEGIN(PauseMenuItemTag, "ApplicationTag")
REFLECT_FIELD(index)
REFLECT_STRUCT_END(PauseMenuItemTag)