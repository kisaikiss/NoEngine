#include "GameProgressComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(CollectibleItemTag, "ApplicationTag")
REFLECT_STRUCT_END(CollectibleItemTag)

REFLECT_STRUCT_BEGIN(GoalItemTag, "ApplicationTag")
REFLECT_STRUCT_END(GoalItemTag)

REFLECT_STRUCT_BEGIN(GameProgressComponent, "Application")
REFLECT_FIELD(totalItemCount),
REFLECT_FIELD(collectedItemCount),
REFLECT_FIELD(elapsedTime),
REFLECT_FIELD(totalCounted)
REFLECT_STRUCT_END(GameProgressComponent)