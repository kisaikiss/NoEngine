#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"


namespace NoEngine {
namespace Component {

// ToDo: ParticleShapeを消し、ParticlePassをモデルパーティクルへ対応させる
enum class ParticleShape {
	kPlane,
	kRing,
	kCylinder
};

struct ParticleComponent {
	Math::Color color = Math::Color::WHITE;
	float lifeTime = 3.f;
	float currentTime = 0.f;
	ParticleShape shape = ParticleShape::kPlane;
	TextureRef texture;			// 使用するテクスチャ
};

struct ParticleEmitterComponent {
	bool active = true;

	// パーティクルの設定
	uint32_t maxCount = 100;	// 最大発生数
	uint32_t count = 10;		// 発生数
	float frequency = 0.5f;		// 発生頻度
	float frequencyTime = 0.0f; // 頻度用時刻
	Math::Vector3 maxSpeed = Math::Vector3::UNIT_SCALE;
	Math::Vector3 minSpeed = -Math::Vector3::UNIT_SCALE;
	Math::Vector3 maxScale = Math::Vector3::UNIT_SCALE;
	Math::Vector3 minScale = Math::Vector3::ZERO;
	Math::Vector3 emitRange = Math::Vector3::UNIT_SCALE;
	Math::Vector3 localPosition = Math::Vector3::ZERO;
	Math::Color color = Math::Color::WHITE;
	float maxLifeTime = 6.f;
	float minLifeTime = 2.f;
	ParticleShape shape = ParticleShape::kPlane;
	std::string textureName = "white1x1";
	TextureRef texture;			// 使用するテクスチャ
};

struct ParticleEmitterSphereComponent {
	float radius = 1.0f;
	uint32_t count = 10;
	float frequency = 0.5f;
	float frequencyTime = 0.0f;
	uint32_t emit = 0;
	
};

}
}