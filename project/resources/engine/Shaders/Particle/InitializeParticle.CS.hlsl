
#include "Particle.hlsli"
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int> gFreeCounter : register(u1);

static uint kMaxParticle = 1024;
[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint particleIndex = DTid.x;
    if (particleIndex == 0)
    {
        gFreeCounter[0] = 0;
    }
    
    if (particleIndex < kMaxParticle)
    {
        gParticles[particleIndex] = (Particle)0;

    }
} 