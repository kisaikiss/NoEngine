#pragma once
#include "ISystem.h"
#include "externals/nlohmann/json.hpp"
namespace NoEngine {
namespace ECS {
class EditSystem :
    public ISystem {
public:
    EditSystem() { SetStopInGameStop(false); }
    void Update(Registry& registry, float deltaTime) override;
private:
    void SaveFile(Registry& registry, nlohmann::json j);
    void LoadFile(Registry& registry);

    void DrawHierarchyWindow(Registry& registry);
    void RenameEditTag(Registry& registry, Entity entity, const std::string& desiredName);
    bool FirstLoaded_ = false;
};
}
}
