#pragma once
// [Deprecated] このヘッダーは後方互換性のために残してあります。
// 新しいコードは application/CommentBout/Data/PlayerConfig.h と
// application/CommentBout/Data/PlayerDataIO.h を使用してください。
#include "application/CommentBout/Data/PlayerConfig.h"
#include "application/CommentBout/Data/PlayerDataIO.h"
#include <fstream>

// 後方互換: 旧名 PlayerConfigComponent を新名 PlayerConfig のエイリアスとして提供
using PlayerConfigComponent = PlayerConfig;

// 後方互換: 旧関数を PlayerDataIO のラッパーとして提供
inline std::string MakePlayerConfigFilePath() {
    return PlayerDataIO::kDefaultPath;
}

inline bool LoadPlayerConfig(PlayerConfig& outConfig) {
    outConfig = PlayerDataIO::Load();
    return true;
}

inline bool SavePlayerConfig(const PlayerConfig& config) {
    PlayerDataIO::Save(config);
    return true;
}
