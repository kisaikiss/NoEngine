#pragma once
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Runtime/Command/ComputeContext.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
__declspec(align(16))struct Particle {
    Math::Vector3 translate;
    float pad;
    Math::Vector3 scale;
    float lifeTime;
    Math::Vector3 velocity;
    float currentTime;
    Math::Vector4 color;
};


class ParticleManager {
public:
	static void Initialize(ComputeContext& ctx);
    static void Reset(ComputeContext& ctx);
	static void Shutdown();

	static StructuredBuffer& GetParticleBuffer();
    static StructuredBuffer& GetFreeListIndexBuffer();
    static StructuredBuffer& GetFreeListBuffer();
    static uint32_t GetParticleNum();
};
}

