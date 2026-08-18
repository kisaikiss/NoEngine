#pragma once
#include <cstdint>

// GameScene -> GameClearSceneへ結果を橋渡しするための一時置き場。
// IScene::registry_はシーンごとに独立しているため、Entity/Componentでは
// 次のシーンへ値を持ち越せない。staticな値でそのまま受け渡す。
namespace GameResult {
struct Data {
	uint32_t totalItemCount = 0;
	uint32_t collectedItemCount = 0;
	float clearTime = 0.0f;
};

void Set(const Data& data);
const Data& Get();
}