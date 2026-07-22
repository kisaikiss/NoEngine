#include "Particle.hlsli"
#include "../Random.hlsli"
RWStructuredBuffer<Particle> gParticles : register(u0);

struct PerFrame
{
    float time;
    float deltaTime;
    float2 pad;
};
ConstantBuffer<PerFrame> gPerFrame : register(b0);

static const int kMaxParticles = 1024;

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    if (particleIndex < kMaxParticles)
    {
        if (gParticles[particleIndex].color.a != 0)
        {
            gParticles[particleIndex].translate += gParticles[particleIndex].velocity;
            gParticles[particleIndex].currentTime += gPerFrame.deltaTime;
            float alpha = 1.0f - (gParticles[particleIndex].currentTime / gParticles[particleIndex].lifeTime);
            gParticles[particleIndex].color.a = saturate(alpha);
        }
    }
}