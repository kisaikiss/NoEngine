Texture2D<float4> gFog : register(t0);

float4 main(float4 position : SV_POSITION) : SV_TARGET0
{
    float4 fog = gFog.Load(int3(int2(position.xy), 0));
    return float4(fog.rgb, 1.0 - saturate(fog.a));
}
