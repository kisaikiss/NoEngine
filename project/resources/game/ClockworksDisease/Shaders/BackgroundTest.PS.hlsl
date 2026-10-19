#include "BackgroundCommon.hlsli"

float2 mod(float2 x, float y)
{
    return x - y * floor(x / y);
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float3 c;
    float l, z = iTime;
    [loop]
    for (int i = 0; i < 3; i++)
    {
        float2 uv, p = fragCoord.xy / iResolution.xy;
        uv = p;
        p -= .5;
        p.x *= iResolution.x / iResolution.y;
        z += .07;
        l = length(p);
        uv += p / l * (sin(z) + 1.) * abs(sin(l * 9. - z - z));
        c[i] = .01 / length(mod(uv, 1.) - .5);
    }
    fragColor = float4(c / l, iTime);
}