#pragma once

/// <summary>
/// 敵撃破時の報酬生成制御。
/// 同一敵から複数回オーブが出ないよう spawned でガードする
/// </summary>
struct EnemyRewardSourceComponent {
	float worldSizeForReward = 1.0f;
	bool spawned = false;
};
