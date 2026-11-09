#pragma once
#include "engine/Math/MathInclude.h"
#include "ParticleEmitterComponent.h"

namespace NoEngine {
namespace Component {

// 保存済みのシーンと Prefab 用。
struct EffectEmitTag{};

enum class EffectSpawnShape {
    kPoint,
    kBox,
    kDiskXZ,
    kRingXZ,
    kSphere
};

enum class EffectVelocityDistribution {
    kBox,
    kDiskXZ,
    kDiskXY,
    kRingXZ,
    kSphere
};

struct EffectEmitterComponent {
    float startDelay = 0.0f;      // 初回発生までの秒数
    uint32_t burstCount = 1;    // 発生の回数
    float burstInterval = 0.1f;  // 繰り返しの間隔（秒）
    uint32_t count = 10;        // 1 回の発生数
    EffectSpawnShape spawnShape = EffectSpawnShape::kBox;
    Math::Vector3 spawnOffset = Math::Vector3::ZERO;
    Math::Vector3 emitRange = Math::Vector3::ZERO;
    float spawnRadius = 1.0f;
    EffectVelocityDistribution velocityDistribution = EffectVelocityDistribution::kBox;
    Math::Vector3 maxSpeed = Math::Vector3::UNIT_SCALE;
    Math::Vector3 minSpeed = -Math::Vector3::UNIT_SCALE;
    Math::Vector3 maxScale = Math::Vector3::UNIT_SCALE;
    Math::Vector3 minScale = Math::Vector3::ZERO;
    Math::Color color = Math::Color::WHITE;
    float maxLifeTime = 6.f;
    float minLifeTime = 2.f;
    float maxRotate = PI;
    float minRotate = 0.f;
    ParticleShape shape = ParticleShape::kPlane;
    std::string textureName = "white1x1";
    TextureRef texture;			// 使用するテクスチャ
};

using EffectPreset = EffectEmitterComponent;
}
}
