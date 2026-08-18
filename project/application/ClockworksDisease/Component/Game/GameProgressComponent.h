#pragma once
#include "engine/NoEngine.h"

// 収集対象アイテムに付けるタグ。総数カウントと取得数カウントの両方に使う。
// 既存のPowerItemComponent/BigPowerItemComponentを持つEntityに追加で付ける想定（エディタのAdd Componentから）。
struct CollectibleItemTag {};

// ゴールアイテムに付けるタグ。ItemGetSystemがこれを見てゲームクリア判定を行う。
struct GoalItemTag {};

// シーン内のアイテム総数・取得数・経過時間を保持する。
// GameScene::Setup()で専用の管理Entity(1つだけ)に付ける。
struct GameProgressComponent {
	uint32_t totalItemCount = 0;
	uint32_t collectedItemCount = 0;
	float elapsedTime = 0.0f;
	bool totalCounted = false; // 総アイテム数を数え終えたか（GameProgressInitSystemが使用）
};