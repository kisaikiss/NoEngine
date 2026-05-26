#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "../ParticleEmitterComponent.h"

namespace NoEngine {
namespace Component {
struct EffectEmitTag{};

struct EffectEmitterComponent {
    uint32_t count = 10;		// 発生数
    Math::Vector3 maxSpeed = Math::Vector3::UNIT_SCALE;
    Math::Vector3 minSpeed = -Math::Vector3::UNIT_SCALE;
    Math::Vector3 maxScale = Math::Vector3::UNIT_SCALE;
    Math::Vector3 minScale = Math::Vector3::ZERO;
    float maxLifeTime = 6.f;
    float minLifeTime = 2.f;
    float maxRotate = PI;
    float minRotate = 0.f;
    ParticleShape shape = ParticleShape::kPlane;
    TextureRef texture;			// 使用するテクスチャ
};
}
}