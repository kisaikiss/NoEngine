#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Functions/ECS/Entity.h"

namespace NoEngine {
	namespace ECS {
		class Registry;
	}
}

/// <summary>
/// 撃破演出の個別パーティクルコンポーネント
/// </summary>
struct DeathParticleComponent {
	NoEngine::Math::Vector3 startPosition;		// 開始位置
	NoEngine::Math::Vector3 targetPosition;		// 目標位置
	float timer;								// 現在の経過時間
	float duration;								// 演出時間
	NoEngine::Math::Color startColor;			// 開始色
	float startScale;							// 開始スケール

	DeathParticleComponent()
		: startPosition(NoEngine::Math::Vector3::ZERO),
		targetPosition(NoEngine::Math::Vector3::ZERO),
		timer(0.0f),
		duration(0.6f),
		startColor({ 1.0f, 0.2f, 0.2f, 1.0f }),
		startScale(0.15f)
	{
	}
};

/// <summary>
/// 撃破演出設定
/// 演出を生成するために必要なパラメータを保持
/// </summary>
struct DeathEffectConfig {
	NoEngine::Math::Color color;				// パーティクルの色
	float particleScale;						// パーティクルのスケール
	int particleCount;							// パーティクルの数
	float duration;								// 演出時間（秒）
	float spreadDistance;						// 広がる距離

	DeathEffectConfig()
		: color({ 1.0f, 0.2f, 0.2f, 1.0f }),	// 赤
		particleScale(0.15f),
		particleCount(8),
		duration(0.6f),
		spreadDistance(2.0f)
	{
	}
};

/// <summary>
/// 撃破演出を生成するヘルパー
/// </summary>
namespace DeathEffectHelper {
	/// <summary>
	/// 撃破演出を生成
	/// </summary>
	/// <param name="registry">レジストリ</param>
	/// <param name="centerPos">中心位置</param>
	/// <param name="config">演出設定</param>
	void SpawnDeathEffect(NoEngine::ECS::Registry& registry, const NoEngine::Math::Vector3& centerPos, const DeathEffectConfig& config);
}
