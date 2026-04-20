#pragma once
#include "../Entity.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
struct PotentialPair {
    ECS::Entity a;
    ECS::Entity b;
};

struct Contact {
    ECS::Entity a;
    ECS::Entity b;
    Math::Vector3 normal;      // b → a の押し戻し方向
    float penetration;
};

namespace Event {
struct PotentialPairEvent {
    std::vector<PotentialPair> pairs;
};

struct ContactEvent {
    std::vector<Contact> contacts;
};

}
}