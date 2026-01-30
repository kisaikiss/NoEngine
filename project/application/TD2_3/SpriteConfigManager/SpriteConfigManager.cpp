#include "SpriteConfigManager.h"
#include"../../../externals/nlohmann/json.hpp"


nlohmann::json SpriteToJson(const NoEngine::Component::SpriteComponent& sp)
{
    return {
        {"pivot", {sp.pivot.x, sp.pivot.y}},
        {"flipX", sp.flipX},
        {"flipY", sp.flipY},
        {"uv", {sp.uv.x, sp.uv.y}},
        {"layer", sp.layer},
        {"orderInLayer", sp.orderInLayer},
        {"color", {sp.color.r, sp.color.g, sp.color.b, sp.color.a}}
    };
}

//nlohmann::json SpriteToJson(const NoEngine::Component::Transform2DComponent& sp)
//{
//    return {
//        {"translate", {sp.translate.x, sp.translate.y}},
//        {"scale", sp.scale.x,sp.scale.y},
//    };
//}


void ApplyJsonToSprite( NoEngine::Component::SpriteComponent& sp, const nlohmann::json& j)
{

    if (j.contains("pivot")) {
        sp.pivot.x = j["pivot"][0];
        sp.pivot.y = j["pivot"][1];
    }
    if (j.contains("flipX")) sp.flipX = j["flipX"];
    if (j.contains("flipY")) sp.flipY = j["flipY"];

    if (j.contains("uv")) {
        sp.uv.x = j["uv"][0];
        sp.uv.y = j["uv"][1];
    }

    if (j.contains("layer")) sp.layer = j["layer"];
    if (j.contains("orderInLayer")) sp.orderInLayer = j["orderInLayer"];

    if (j.contains("color")) {
        sp.color.r = j["color"][0];
        sp.color.g = j["color"][1];
        sp.color.b = j["color"][2];
        sp.color.a = j["color"][3];
    }
}

void ApplyJsonToTransform(NoEngine::Component::Transform2DComponent& t, const nlohmann::json& j)
{
    if (j.contains("translate")) {
        t.translate.x = j["translate"][0];
        t.translate.y = j["translate"][1];
    }

    if (j.contains("rotate")) {
        t.rotation = j["rotate"];
    }

    if (j.contains("scale")) {
        t.scale.x = j["scale"][0];
        t.scale.y = j["scale"][1];
    }
  
}

void SpriteConfigManager::Load(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return;
    f >> jsonData;

}

void SpriteConfigManager::ApplyToSprite(NoEngine::Component::SpriteComponent& sp,NoEngine::Component::Transform2DComponent& t)
{

    if (sp.name.empty()) return;
    if (!jsonData.contains(sp.name)) return;

    const auto& j = jsonData[sp.name];

    ApplyJsonToSprite(sp, j);

    ApplyJsonToTransform(t, j); 

}
