#pragma once
#include "../Entity.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace Math {
/// <summary>
/// 衝突時の法線の向き
/// </summary>
enum class ContactPosition {
    UP,
    DOWN,
    SIDE
};

}

struct PotentialPair {
    ECS::Entity a;
    ECS::Entity b;
};

struct Contact {
    ECS::Entity a;
    ECS::Entity b;
    Math::Vector3 normal;      // b → a の方向
    float penetration;
    Math::ContactPosition contactPosition;          // bから見たaの位置
};

struct Contact2D {
    ECS::Entity a;
    ECS::Entity b;
    Math::Vector2 normal;      // b → a の方向
    float penetration;
    Math::ContactPosition contactPosition;          // bから見たaの位置
};

namespace Event {
struct PotentialPairEvent {
    std::vector<PotentialPair> pairs;
};

struct ContactEvent {
    std::vector<Contact> contacts;
};

struct Contact2DEvent {
    std::vector<Contact2D> contacts;
};

}
}