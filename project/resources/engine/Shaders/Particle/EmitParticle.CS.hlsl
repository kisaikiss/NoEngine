#include "Particle.hlsli"
#include "../Random.hlsli"
RWStructuredBuffer<Particle> gParticles : register(u0);

struct EmittterSphere
{
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

RWStructuredBuffer<int> gFreeCounter : register(u1);

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
void main( uint3 DTid : SV_DispatchThreadID )
{
    if (gEmitter.emit != 0)
    {
        RandomGenerator generator;
        generator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
        
        for (uint countIndex = 0; countIndex < gEmitter.count; ++countIndex)
        {
            int particleIndex;
            InterlockedAdd(gFreeCounter[0], 1, particleIndex);
            
            if (particleIndex < kMaxParticles)
            {
                // カウント分Particleを射出する
                gParticles[particleIndex].scale = generator.Generate3d();
                gParticles[particleIndex].translate = generator.Generate3d();
                gParticles[particleIndex].color = float4(generator.Generate3d(), 1.0f);
                
            }

        }

    }
}