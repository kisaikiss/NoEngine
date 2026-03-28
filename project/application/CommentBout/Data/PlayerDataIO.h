#pragma once
#include "application/CommentBout/Data/PlayerConfig.h"
#include <string>

/// <summary>
/// PlayerConfig のロード/セーブを担当するIOクラス。
/// </summary>
class PlayerDataIO {
public:
    static constexpr const char* kDefaultPath = "resources/game/td_3105/Data/Config/PlayerConfig.json";

    static PlayerConfig Load(const std::string& path = kDefaultPath);
    static void Save(const PlayerConfig& config, const std::string& path = kDefaultPath);
};
