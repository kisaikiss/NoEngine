#pragma once
#include "ISystem.h"
#include "externals/nlohmann/json.hpp"
namespace NoEngine {
namespace ECS {
class EditSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;
private:
    void SaveFile(Registry& registry, nlohmann::json j);
    void LoadFile(Registry& registry);

    bool FirstLoaded_ = false;
};
}
}
