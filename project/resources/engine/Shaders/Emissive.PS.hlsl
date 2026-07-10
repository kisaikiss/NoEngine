#include "Default.hlsli"

struct EmissiveMaterial
{
    float4 color; // ベースカラー（発光色）
    float intensity; // 発光強度
    float rimPower; // フレネル(縁)の鋭さ
    float scrollSpeed; // ノイズのスクロール速度
    float time; // 経過時間
};
ConstantBuffer<EmissiveMaterial> gEmissive : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gNoiseTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = 0;

    float3 normal = normalize(input.normal);
    float3 toEye = normalize(gCameraMatrix.worldPosition - input.worldPosition);

    // フレネル項：視線と法線のなす角が浅いほど（=縁ほど）強く光る
    float NdotV = saturate(dot(normal, toEye));
    float fresnel = pow(1.0f - NdotV, gEmissive.rimPower);

    // ノイズを2枚、逆方向・異なる速度でスクロールさせて掛け合わせ、プラズマの揺らぎを作る
    float2 uv1 = input.texcoord + float2(gEmissive.time * gEmissive.scrollSpeed, gEmissive.time * gEmissive.scrollSpeed * 0.5f);
    float2 uv2 = input.texcoord * 1.7f - float2(gEmissive.time * gEmissive.scrollSpeed * 0.6f, gEmissive.time * gEmissive.scrollSpeed * 0.9f);

    float noiseA = gNoiseTexture.Sample(gSampler, uv1).r;
    float noiseB = gNoiseTexture.Sample(gSampler, uv2).r;
    float noise = saturate(noiseA * noiseB * 1.5f); // 掛け合わせで筋状のプラズマ模様に

    // フレネルとノイズをブレンドしてコアとエッジの両方が光るようにする
    float glow = saturate(fresnel * 0.6f + noise * 0.7f);

    output.color.rgb = gEmissive.color.rgb * gEmissive.intensity * glow;
    output.color.a = 1.0f; // 加算合成なのでアルファは実質使わないが1.0にしておく

    return output;
}