#pragma once
#include "externals/nlohmann/json.hpp"
#include "engine/Functions/ECS/Registry.h"
#include "../ComponentRegistry.h"

namespace NoEngine {
namespace Editor {
nlohmann::json SaveScene(ECS::Registry& registry);
nlohmann::json SaveEntityToJson(ECS::Registry& registry, ECS::Entity entity);
void WriteFieldToJson(nlohmann::json& j, const FieldInfo& field, void* ptr);

// 配列要素1個をJSON化／復元する（Struct要素・プリミティブ要素どちらも対応）
// DrawFieldUIのUndo用スナップショットにも流用する
nlohmann::json WriteArrayElementToJson(const FieldInfo& arrayField, void* elemPtr);
void ReadArrayElementFromJson(const nlohmann::json& elemJson, const FieldInfo& arrayField, void* elemPtr);

void LoadScene(ECS::Registry& registry, const nlohmann::json& scene);
ECS::Entity FindEntityByName(ECS::Registry& registry, const std::string& name);
void LoadEntityFromJson(ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& j);
void ReadFieldFromJson(const nlohmann::json& j, const FieldInfo& field, void* ptr);
}
}