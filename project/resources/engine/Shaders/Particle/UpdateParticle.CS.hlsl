#include "Particle.hlsli"
RWStructuredBuffer<Particle> gParticles : register(u0);

RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);
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
    if (particleIndex >= kMaxParticles)
    {
        return;
    }
    if (gParticles[particleIndex].active == 0)
    {
        return;
    }

    gParticles[particleIndex].translate += gParticles[particleIndex].velocity;
    float currentTime = gParticles[particleIndex].currentTime + gPerFrame.deltaTime;
    gParticles[particleIndex].currentTime = currentTime;
    float lifeTime = gParticles[particleIndex].lifeTime;
    if (currentTime >= lifeTime)
    {
        // 生存状態から切り替わったときだけ空きリストへ戻す。
        gParticles[particleIndex].active = 0;
        gParticles[particleIndex].color.a = 0.0f;
        gParticles[particleIndex].scale = float3(0.0f, 0.0f, 0.0f);
        int freeListIndex;
        InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
        if ((freeListIndex + 1) < kMaxParticles)
        {
            gFreeList[freeListIndex + 1] = particleIndex;
        }
        else
        {
            InterlockedAdd(gFreeListIndex[0], -1);
        }
        return;
    }

    gParticles[particleIndex].color.a = saturate(1.0f - currentTime / lifeTime);
}
