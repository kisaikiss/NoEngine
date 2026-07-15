
#include "Particle.hlsli"
RWStructuredBuffer<Particle> gParticles : register(u0);

static uint kMaxParticle = 1024;
[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint particleIndex = DTid.x;
    if (particleIndex < kMaxParticle)
    {
        gParticles[particleIndex] = (Particle)0;
        // テスト描画用に見える値を入れる
        gParticles[particleIndex].scale = float3(0.5f, 0.5f, 0.5f);
        gParticles[particleIndex].color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    }
} 