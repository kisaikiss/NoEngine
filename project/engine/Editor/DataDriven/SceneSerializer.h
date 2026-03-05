#pragma once
#include "externals/nlohmann/json.hpp"
#include "engine/Functions/ECS/Registry.h"
#include "../ComponentRegistry.h"

namespace NoEngine {
namespace Editor {
nlohmann::json SaveScene(ECS::Registry& registry);
nlohmann::json SaveEntityToJson(ECS::Registry& registry, ECS::Entity entity);
void WriteFieldToJson(nlohmann::json& j, const FieldInfo& field, void* ptr);

void LoadScene(ECS::Registry& registry, const nlohmann::json& scene);
ECS::Entity FindEntityByName(ECS::Registry& registry, const std::string& name);
void LoadEntityFromJson(ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& j);
void ReadFieldFromJson(const nlohmann::json& j, const FieldInfo& field, void* ptr);
}
}