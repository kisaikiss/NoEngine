#pragma once
#include "application/CommentBout/Data/EnemyConfig.h"
#include <string>

/// <summary>
/// EnemyConfigMap のロード/セーブを担当するIOクラス。
/// </summary>
class EnemyDataIO {
public:
    static constexpr const char* kDefaultPath = "resources/game/td_3105/Data/Config/EnemyConfig.json";

    static EnemyConfigMap Load(const std::string& path = kDefaultPath);
    static void Save(const EnemyConfigMap& configs, const std::string& path = kDefaultPath);

    static EnemyConfig GetOrDefault(const EnemyConfigMap& configs, RailEnemyType type);
};
