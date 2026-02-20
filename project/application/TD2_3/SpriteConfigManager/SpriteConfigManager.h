#pragma once
#include"../../../externals/nlohmann/json.hpp"
#include"../../../engine/Functions/ECS/Component/SpriteComponent.h"
#include"../../../engine/Functions/ECS/Component/Transform2DComponent.h"

nlohmann::json SpriteToJson(const  NoEngine::Component::SpriteComponent& sp);
void ApplyJsonToSprite(NoEngine::Component::SpriteComponent& sp, const nlohmann::json& j);
void ApplyJsonToTransform(NoEngine::Component::Transform2DComponent& t, const nlohmann::json& j);

class SpriteConfigManager {

public:

    static SpriteConfigManager& Get() { 
        static SpriteConfigManager instance;
        return instance;
    }

    void Load(const std::string& path);
    void ApplyToSprite(NoEngine::Component::SpriteComponent& sp, NoEngine::Component::Transform2DComponent& t);
public:
    nlohmann::json jsonData;
private: 
    SpriteConfigManager() = default;
};
