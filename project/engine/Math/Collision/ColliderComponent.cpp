#include "ColliderComponent.h"
#include "engine/Editor/ReflectionMacros.h"

REFLECT_STRUCT_BEGIN(NoEngine::Math::AABBCollider)
REFLECT_FIELD(min),
REFLECT_FIELD(max)
REFLECT_STRUCT_END(NoEngine::Math::AABBCollider)

REFLECT_STRUCT_BEGIN(NoEngine::Math::AABBCollider2D)
REFLECT_FIELD(min),
REFLECT_FIELD(max)
REFLECT_STRUCT_END(NoEngine::Math::AABBCollider2D)

REFLECT_STRUCT_BEGIN(NoEngine::Math::CapsuleCollider)
REFLECT_FIELD(localP0),
REFLECT_FIELD(localP1),
REFLECT_FIELD(radius)
REFLECT_STRUCT_END(NoEngine::Math::CapsuleCollider)


REFLECT_STRUCT_BEGIN(NoEngine::Math::SphereCollider)
REFLECT_FIELD(localCenter),
REFLECT_FIELD(radius)
REFLECT_STRUCT_END(NoEngine::Math::SphereCollider)


REFLECT_STRUCT_BEGIN(NoEngine::Component::GroundStateComponent)
REFLECT_FIELD(isGrounded),
REFLECT_FIELD(groundHeight)
REFLECT_STRUCT_END(NoEngine::Component::GroundStateComponent)


REFLECT_STRUCT_BEGIN(NoEngine::Component::CollisionBody)
REFLECT_FIELD(type)
REFLECT_STRUCT_END(NoEngine::Component::CollisionBody)

