#pragma once
#include "../CommandManager.h"
#include "engine/Functions/ECS/Registry.h"

#include "externals/nlohmann/json.hpp"


namespace NoEngine {
namespace Command {
class AddComponentCommand : public ICommand {
public:
    AddComponentCommand(ECS::Registry& registry, ECS::Entity entity, size_t typeID);
    void Execute() override;
    void Undo() override;
private:
    ECS::Registry& registry_;
    ECS::Entity entity_;
    size_t typeID_;
};
}
}

