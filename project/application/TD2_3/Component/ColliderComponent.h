#pragma once
#include "engine/Math/MathInclude.h"

enum ColliderMask
{
	kBall = 1 << 0,
	kEnemy = 1 << 1,
};

struct SphereColliderComponent
{
    NoEngine::Vector3 center;
    float radius = 0.5f;
    float worldRadius = 0.5f;
    bool isCollied = false;
    uint32_t colliderType = 0xffffffff;
	uint32_t collideMask = 0xffffffff;
	ColliderMask colliedWith = static_cast<ColliderMask>(0);
};

struct BoxColliderComponent
{
    NoEngine::Vector3 center;
    NoEngine::Vector3 size = { 1.0f, 1.0f, 1.0f };
    NoEngine::Vector3 worldSize = { 1.0f, 1.0f, 1.0f };
    bool isCollied = false;
    uint32_t colliderType = 0xffffffff;
    uint32_t collideMask = 0xffffffff;
};
