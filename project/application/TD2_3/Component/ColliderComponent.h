#pragma once
#include "engine/Math/MathInclude.h"

struct SphereColliderComponent
{
    NoEngine::Vector3 center;
    float radius = 0.5f;
    float worldRadius = 0.5f;
    bool isCollied = false;
};

struct BoxColliderComponent
{
    NoEngine::Vector3 center;
    NoEngine::Vector3 size = { 1.0f, 1.0f, 1.0f };
    NoEngine::Vector3 worldSize = { 1.0f, 1.0f, 1.0f };
    bool isCollied = false;
};
