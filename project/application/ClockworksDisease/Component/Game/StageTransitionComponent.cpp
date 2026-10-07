#include "StageTransitionComponent.h"

REFLECT_STRUCT_BEGIN(StageTransitionComponent, "Application")
REFLECT_FIELD(destinationScene),
REFLECT_FIELD(transitionTime),
REFLECT_GIZMO_FIELD(returnPosition)
REFLECT_STRUCT_END(StageTransitionComponent)
