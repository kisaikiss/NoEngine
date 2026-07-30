#include "Particle.hlsli"
#include "../Random.hlsli"
RWStructuredBuffer<Particle> gParticles : register(u0);

struct EmittterSphere
{
    float3 position;
    float radius;
    uint count;
    uint emit;
    uint2 pad;
};
ConstantBuffer<EmittterSphere> gEmitter : register(b0);

struct PerFrame
{
    float time;
    float deltaTime;
    float2 pad;
};
ConstantBuffer<PerFrame> gPerFrame : register(b1);

RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

class RandomGenerator
{
    float3 seed;
    float3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
    
    float Generate1d()
    {
        float result = rand3dTo1d(seed);
        seed.x = result;
        return result;
    }
};

static const int kMaxParticles = 1024;

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (gEmitter.emit != 0)
    {
        RandomGenerator generator;
        generator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
        
        for (uint countIndex = 0; countIndex < gEmitter.count; ++countIndex)
        {
            int freeListIndex;
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            if (0 <= freeListIndex && freeListIndex < kMaxParticles)
            {
                uint particleIndex = gFreeList[freeListIndex];
                // カウント分Particleを射出する
                gParticles[particleIndex].scale = generator.Generate3d();
                float3 dir = normalize(generator.Generate3d() * 2.0f - 1.0f);
                float r = pow(generator.Generate1d(), 1.0f / 3.0f) * gEmitter.radius;
                gParticles[particleIndex].translate = gEmitter.position + dir * r;
                gParticles[particleIndex].color = float4(generator.Generate3d(), 1.0f);
                gParticles[particleIndex].lifeTime = generator.Generate1d();
                gParticles[particleIndex].currentTime = 0.0f;
                gParticles[particleIndex].velocity = 0.1f - generator.Generate3d() / 5.0f;
            }
            else
            {
                // 発生させられなかったので減らした分を元に戻す。
                InterlockedAdd(gFreeListIndex[0], 1);
                break;
            }

        }

    }
}