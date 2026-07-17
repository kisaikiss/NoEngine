#pragma once
#include "engine/NoEngine.h"

struct MagicScaffoldComponent {
    No::Vector3 finalSize = No::Vector3(3.0f, 0.7f, 3.0f);
    float time = 0.0f;
    bool isDead = false;
    uint32_t generation = 0;
};

class MagicScaffoldSystem :
    public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
};