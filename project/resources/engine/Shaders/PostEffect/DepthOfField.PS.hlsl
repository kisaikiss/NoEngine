Texture2D<float4> gTexture : register(t0);
Texture2D<float> gDepthTexture : register(t1);
SamplerState gSampler : register(s0);
SamplerState gSamplerPoint : register(s1);

struct Material
{
    float4x4 projectionInverse;
    float focusDistance; // 合焦距離（ビュー空間 Z, 正の値）
    float focusRange;    // ここより離れると徐々にボケ始める幅
    float maxCoCRadius;  // 最大ボケ半径（UV空間, 例: 0.01）
    float padding;
};
ConstantBuffer<Material> gMaterial : register(b0);

// 12サンプルのポアソンディスク（円形ボケ用のサンプリングパターン）
static const float2 kPoissonDisk[12] =
{
    float2(-0.326f, -0.406f),
    float2(-0.840f, -0.074f),
    float2(-0.696f, 0.457f),
    float2(-0.203f, 0.621f),
    float2(0.962f, -0.195f),
    float2(0.473f, -0.480f),
    float2(0.519f, 0.767f),
    float2(0.185f, -0.893f),
    float2(0.507f, 0.064f),
    float2(0.896f, 0.412f),
    float2(-0.322f, -0.933f),
    float2(-0.792f, -0.598f)
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

// DepthBasedOutlinePSと同じ手法でNDC深度からビュー空間Zを復元
float ReconstructViewZ(float2 texcoord)
{
    float ndcDepth = gDepthTexture.Sample(gSamplerPoint, texcoord);
    float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gMaterial.projectionInverse);
    return viewSpace.z * rcp(viewSpace.w);
}

// 円錯乱径(CoC)を算出。0=完全合焦、1=最大ボケ
float ComputeCoC(float viewZ)
{
    float diff = abs(viewZ - gMaterial.focusDistance);
    float coc = saturate((diff - gMaterial.focusRange) / max(gMaterial.focusRange, 1e-4f));
    return coc;
}

PSOutput main(VSOutput input)
{
    float centerViewZ = ReconstructViewZ(input.texcoord);
    float coc = ComputeCoC(centerViewZ);

    float3 sharpColor = gTexture.Sample(gSampler, input.texcoord).rgb;

    float radius = coc * gMaterial.maxCoCRadius;
    float3 blurredColor = float3(0.0f, 0.0f, 0.0f);

    [unroll]
    for (int i = 0; i < 12; ++i)
    {
        float2 sampleUV = input.texcoord + kPoissonDisk[i] * radius;
        blurredColor += gTexture.Sample(gSampler, sampleUV).rgb;
    }
    blurredColor *= rcp(12.0f);

    PSOutput o;
    o.color.rgb = lerp(sharpColor, blurredColor, coc);
    o.color.a = 1.0f;
    return o;
}
