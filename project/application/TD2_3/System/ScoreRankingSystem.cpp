#include "ScoreRankingSystem.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/ScoreDigitComponent.h"
#include "engine/Math/Easing.h"
#include "externals/imgui/imgui.h"

#include <algorithm>

using namespace NoEngine;

namespace
{
	NoEngine::Vector2 skRankingBase = { 800.0f, 500.0f };
	const float skSpriteWidth = 64.f;
	const uint32_t skDigitNum = 6;

	// ランク行間、高さオフセット（開始位置 = target + skStartOffsetY）
	const float skRankingSpacingY = 96.0f;
	const float skStartOffsetY = 400.f;

	// 各ランクが開始するまでの遅延（低スコアから順に表示されるように増やす）
	const float skPerRankDelay = 0.25f;

	const NoEngine::Color kGold = NoEngine::Color(0xD0A900FF);
	//const NoEngine::Color kGoldHighlight = NoEngine::Color(0xfff9e6ff);

	const NoEngine::Color kSilver = NoEngine::Color(0.75f, 0.75f, 0.75f, 1.0f);
	const NoEngine::Color kBronze = NoEngine::Color(0.804f, 0.498f, 0.196f, 1.0f);
}

static float EaseOutSmooth(float t)
{
	if (t <= 0.f) return 0.f;
	if (t >= 1.f) return 1.f;
	return t * t * (3.0f - 2.0f * t);
}

void ScoreRankingSystem::Update(No::Registry& registry, float deltaTime)
{
	// デバッグでベース位置を調整可能に
#ifdef USE_IMGUI
	ImGui::Begin("ranking debug");
	ImGui::DragFloat2("rankingBase", &skRankingBase.x);
	ImGui::End();
#endif // USE_IMGUI

	// 現在のプレイヤースコアを取得
	auto playerView = registry.View<PlayerStatusComponent>();
	int32_t currentScore = 0;
	for (auto ent : playerView)
	{
		auto* status = registry.GetComponent<PlayerStatusComponent>(ent);
		currentScore = status->score;
	}

	if (!scoresCommitted_)
	{
		std::array<int32_t, 4> tmp;
		tmp[0] = ScoreRankingSystem::topScores_[0];
		tmp[1] = ScoreRankingSystem::topScores_[1];
		tmp[2] = ScoreRankingSystem::topScores_[2];
		tmp[3] = currentScore;

		// 降順ソートして上位3件を取り出す
		std::sort(tmp.begin(), tmp.end(), std::greater<int32_t>());

		// 内部は昇順（index0 = 最低, index2 = 最高）で保持する
		ScoreRankingSystem::topScores_[0] = tmp[2]; // 3番目（最低）
		ScoreRankingSystem::topScores_[1] = tmp[1]; // 2番目
		ScoreRankingSystem::topScores_[2] = tmp[0]; // 1番目（最高）

		scoresCommitted_ = true;
	}

	// ランク用の数字スプライトを初回に作成する
	if (rankDigitEntities_.empty())
	{
		rankDigitEntities_.reserve(3 * skDigitNum);
		for (uint32_t rank = 0; rank < 3; ++rank)
		{
			for (uint32_t d = 0; d < skDigitNum; ++d)
			{
				auto ent = registry.GenerateEntity();
				// スプライト
				auto* sprite = registry.AddComponent<No::SpriteComponent>(ent);
				sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/numbers.png");
				sprite->uv.width = 0.1f;
				if (rank == 2)       sprite->color = kGold;
				else if (rank == 1)  sprite->color = kSilver;
				else                 sprite->color = kBronze;

				// トランスフォーム2D
				auto* transform = registry.AddComponent<No::Transform2DComponent>(ent);
				transform->scale = { 64.f, 64.f };
				// マーカーコンポーネント
				registry.AddComponent<ScoreDigitComponent>(ent);

				rankDigitEntities_.push_back(ent);
			}
		}

		// 各ランクのアニメーションターゲット/開始位置を初期化
		for (uint32_t r = 0; r < 3; ++r)
		{
			// 表示は下から上へ low->mid->high の順で移動してくるので、
			// target の Y は (baseY + spacing), (baseY), (baseY - spacing)
			float targetY = skRankingBase.y + (1.0f - static_cast<float>(r)) * skRankingSpacingY;
			NoEngine::Vector2 targetBase = { skRankingBase.x, targetY };
			NoEngine::Vector2 startBase = targetBase;
			startBase.y += skStartOffsetY; // 画面下方（ターゲットより下）から上がってくる

			rankAnims_[r].targetBase = targetBase;
			rankAnims_[r].startBase = startBase;
			rankAnims_[r].delay = skPerRankDelay * static_cast<float>(r); // 低スコアから順に遅延を設定
			rankAnims_[r].duration = 0.6f;
			rankAnims_[r].elapsed = 0.f;
			rankAnims_[r].initialized = true;
		}
	}

	// 各ランクのアニメーション更新と数字表示
	for (uint32_t rank = 0; rank < 3; ++rank)
	{
		auto& anim = rankAnims_[rank];
		// アニメ時間を進める
		anim.elapsed += deltaTime;

		// アニメ進行率計算
		float t = 0.f;
		if (anim.elapsed > anim.delay)
		{
			float local = anim.elapsed - anim.delay;
			t = std::clamp(local / anim.duration, 0.f, 1.f);
			t = EaseOutSmooth(t);
		}

		// 対象スコアの各桁を求める
		int32_t s = ScoreRankingSystem::topScores_[rank];
		std::array<uint32_t, skDigitNum> digits{};
		int32_t tmp = s;
		for (uint32_t i = 0; i < skDigitNum; ++i)
		{
			if (tmp <= 0 && i > 0)
			{
				digits[i] = 0;
			}
			else
			{
				digits[i] = static_cast<uint32_t>(std::abs(tmp % 10));
				tmp /= 10;
			}
		}

		// 各桁スプライトの位置更新
		for (uint32_t d = 0; d < skDigitNum; ++d)
		{
			uint32_t index = rank * skDigitNum + d;
			auto ent = rankDigitEntities_[index];
			auto* sprite = registry.GetComponent<No::SpriteComponent>(ent);
			auto* transform = registry.GetComponent<No::Transform2DComponent>(ent);

			// uv設定（数字テクスチャ）
			sprite->uv.x = 0.1f * static_cast<float>(digits[d]);

			// 最終的な桁位置（右寄せするため base - offset）
			NoEngine::Vector2 offset = { static_cast<float>(d) * skSpriteWidth, 0.f };
			NoEngine::Vector2 targetPos = anim.targetBase - offset;
			NoEngine::Vector2 startPos = anim.startBase - offset;

			NoEngine::Vector2 curPos = NoEngine::Easing::EaseInOutBack(startPos, targetPos, t);
			transform->translate = curPos;
		}
	}
}