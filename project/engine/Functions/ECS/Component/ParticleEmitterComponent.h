#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"


namespace NoEngine {
namespace Component {

struct Particle {
	Transform transform;
	Math::Vector3 velocity;
	float lifeTime = 3.f;
	float currentTime = 0.f;
};

struct ParticleEmitterComponent {
	bool active = true;

	// パーティクルの設定
	uint32_t maxCount = 100;	// 最大発生数
	uint32_t count = 10;		// 発生数
	float frequency = 0.5f;		// 発生頻度
	float frequencyTime = 0.0f; // 頻度用時刻
	TextureRef texture;			// 使用するテクスチャ

	// 実行時データ
	std::vector<Particle> particles;
};
}
}