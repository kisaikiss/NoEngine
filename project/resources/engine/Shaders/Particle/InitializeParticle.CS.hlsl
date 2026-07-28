
#include "Particle.hlsli"
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

static uint kMaxParticle = 1024;
[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint particleIndex = DTid.x;
    if (particleIndex == 0)
    {
        gFreeListIndex[0] = kMaxParticle - 1;
    }
    
    if (particleIndex < kMaxParticle)
    {
        gParticles[particleIndex] = (Particle)0;
        gFreeList[particleIndex] = particleIndex;
    }
} 