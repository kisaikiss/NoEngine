#include "AnimatorComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Component::AnimatorComponent, "Asset")
REFLECT_FIELD(currentAnimation),
REFLECT_FIELD(animationSpeedMagnification),
REFLECT_FIELD(drawSkeleton),
REFLECT_FIELD(enableSkinning)
REFLECT_STRUCT_END(NoEngine::Component::AnimatorComponent)
