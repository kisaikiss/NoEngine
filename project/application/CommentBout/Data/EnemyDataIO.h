#pragma once
#include "application/CommentBout/Data/EnemyConfig.h"
#include <string>

/// <summary>
/// EnemyConfigMap のロード/セーブを担当するIOクラス。
/// </summary>
class EnemyDataIO {
public:
    // TODO(Phase3): ファイルパスを RailData/ から Data/Config/ へ移動する
    static constexpr const char* kDefaultPath = "resources/game/td_3105/RailData/EnemyTypePreset.json";

    static EnemyConfigMap Load(const std::string& path = kDefaultPath);
    static void Save(const EnemyConfigMap& configs, const std::string& path = kDefaultPath);

    static EnemyConfig GetOrDefault(const EnemyConfigMap& configs, RailEnemyType type);
};
