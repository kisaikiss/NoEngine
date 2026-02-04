#pragma once
#include "engine/NoEngine.h"
#include <array>
#include <vector>

class ScoreRankingSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;

private:
	// 各ランク（上位3件）のスコア（index 0 = 最低, 2 = 最高）
	static inline std::array<int32_t, 3> topScores_{ 0,0,0 };

	// ランキング表示に使う数字スプライトのエンティティ群（rank * digit）
	std::vector<NoEngine::ECS::Entity> rankDigitEntities_;

	struct RankAnim
	{
		float elapsed = 0.f;
		float duration = 0.6f;
		float delay = 0.f;
		NoEngine::Vector2 startBase = { 0.f, 0.f };
		NoEngine::Vector2 targetBase = { 0.f, 0.f };
		bool initialized = false;
	};
	std::array<RankAnim, 3> rankAnims_;

	bool scoresCommitted_ = false; // 新しいシステムインスタンスで一度だけ現在スコアを追加するフラグ
};