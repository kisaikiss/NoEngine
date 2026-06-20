#pragma once
#include "../CommandManager.h"
#include "engine/Functions/ECS/Registry.h"

#include "externals/nlohmann/json.hpp"


namespace NoEngine {
namespace Command {
class RemoveComponentCommand : public ICommand {
public:
    RemoveComponentCommand(ECS::Registry& registry, ECS::Entity entity, size_t typeID);
    void Execute() override;
    void Undo() override;
private:
    ECS::Registry& registry_;
    ECS::Entity entity_;
    size_t typeID_;
    nlohmann::json snapshotJson_;
};
}
}