#include "StageComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(StageComponent, "Application")
REFLECT_FIELD(stageColliderName)
REFLECT_STRUCT_END(StageComponent)

REFLECT_STRUCT_BEGIN(ColliderUpdateTag, "ApplicationTag")
REFLECT_STRUCT_END(ColliderUpdateTag)