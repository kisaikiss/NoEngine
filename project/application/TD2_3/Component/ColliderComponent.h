#pragma once
#include "engine/Math/MathInclude.h"

enum ColliderMask
{
	kBall = 1 << 0,
	kEnemy = 1 << 1,
};

struct SphereColliderComponent
{
    No::Vector3 center;
    float radius = 0.5f;
    float worldRadius = 0.5f;
    bool isCollied = false;
    uint32_t colliderType = 0xffffffff;
	uint32_t collideMask = 0xffffffff;
	ColliderMask colliedWith = static_cast<ColliderMask>(0);
	NoEngine::ECS::Entity colliedEntity;
};

struct BoxColliderComponent
{
    No::Vector3 center;
    No::Vector3 size = { 1.0f, 1.0f, 1.0f };
    No::Vector3 worldSize = { 1.0f, 1.0f, 1.0f };
    bool isCollied = false;
    uint32_t colliderType = 0xffffffff;
    uint32_t collideMask = 0xffffffff;
};
