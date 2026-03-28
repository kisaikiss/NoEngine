#pragma once
#include "application/CommentBout/Data/PlayerConfig.h"
#include <string>

/// <summary>
/// PlayerConfig のロード/セーブを担当するIOクラス。
/// </summary>
class PlayerDataIO {
public:
    // TODO(Phase3): ファイルパスを RailData/ から Data/Config/ へ移動する
    static constexpr const char* kDefaultPath = "resources/game/td_3105/RailData/PlayerConfig.json";

    static PlayerConfig Load(const std::string& path = kDefaultPath);
    static void Save(const PlayerConfig& config, const std::string& path = kDefaultPath);
};
