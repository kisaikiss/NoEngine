#pragma once
#include "../CommandManager.h"
#include "engine/Functions/ECS/Registry.h"

#include "externals/nlohmann/json.hpp"


namespace NoEngine {
namespace Command {
class DeleteEntityCommand : public ICommand {
public:
    DeleteEntityCommand(ECS::Registry& registry, ECS::Entity entity);
    void Execute() override;
    void Undo() override;
private:
    ECS::Registry& registry_;
    ECS::Entity entity_;
    nlohmann::json snapshotJson_;
};
}
}

