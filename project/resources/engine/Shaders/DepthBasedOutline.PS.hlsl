
Texture2D<float4> gTexture : register(t0);
Texture2D<float> gDepthTexture : register(t1);
SamplerState gSampler : register(s0);
SamplerState gSamplerPoint : register(s1);

struct Material
{
    float4x4 projectionInverse;
    float chalkEnabled;
    float chalkWidth;
    float chalkThreshold;
    float chalkRoughness;
    float3 chalkColor;
    float chalkSurfaceBrightness;
};
ConstantBuffer<Material> gMaterial : register(b0);


static const float kPrewittHorizontalKernel[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f }
};

static const float kPrewittVerticalKernel[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f },
};

static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } }
};

float Luminance(float3 v)
{
    return dot(v, float3(0.2125f, 0.7154f, 0.0721f));
}



struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

float ChalkGrain(float2 p)
{
    return frac(sin(dot(p, float2(127.1f, 311.7f))) * 43758.5453f);
}

float ViewDepth(float depth)
{
    float4 view = mul(float4(0.0f, 0.0f, depth, 1.0f), gMaterial.projectionInverse);
    return abs(view.z / max(abs(view.w), 1e-6f));
}

PSOutput DrawChalk(VSOutput input, float2 uvStep)
{
    float3 scene = gTexture.Sample(gSampler, input.texcoord).rgb;
    float centerDepth = gDepthTexture.Sample(gSamplerPoint, input.texcoord);
    float centerZ = ViewDepth(centerDepth);
    float edge = 0.0f;
    float grain = ChalkGrain(floor(input.pos.xy));
    // 線幅をゆっくり変化させ、時間で点滅しない静止した描き跡にする。
    float stroke = ChalkGrain(floor(input.pos.xy / 7.0f));
    float radius = gMaterial.chalkWidth * lerp(1.0f, 0.75f + 0.5f * stroke, gMaterial.chalkRoughness);
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float sampleDepth = gDepthTexture.Sample(gSamplerPoint, saturate(input.texcoord + float2(x, y) * uvStep * radius));
            // 背景同士は除外し、距離で正規化して遠方でも輪郭を保つ。
            bool centerIsMesh = centerDepth < 0.999999f;
            bool sampleIsMesh = sampleDepth < 0.999999f;
            float sampleZ = ViewDepth(sampleDepth);
            float relativeDifference = abs(centerZ - sampleZ) / max(min(centerZ, sampleZ), 0.1f);
            float boundary = centerIsMesh != sampleIsMesh ? 1.0f :
                smoothstep(gMaterial.chalkThreshold, gMaterial.chalkThreshold * 2.5f, relativeDifference);
            edge = max(edge, (centerIsMesh || sampleIsMesh) ? boundary : 0.0f);
        }
    }
    float powder = lerp(1.0f, smoothstep(0.08f, 0.75f, grain), gMaterial.chalkRoughness);
    float coverage = edge * powder;
    // 黒板背景はそのまま残し、物体の面だけを抑えて白い線を読みやすくする。
    float3 base = centerDepth < 0.999999f ? scene * gMaterial.chalkSurfaceBrightness : scene;
    PSOutput output;
    output.color = float4(lerp(base, gMaterial.chalkColor, coverage), 1.0f);
    return output;
}

PSOutput main(VSOutput input)
{
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStep = float2(rcp(float(width)), rcp(float(height)));
    if (gMaterial.chalkEnabled > 0.5f)
        return DrawChalk(input, uvStep);
    
    
    float2 difference = float2(0.0f, 0.0f);
    
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStep;
            float ndcDepth = gDepthTexture.Sample(gSamplerPoint, texcoord);
    
            float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), gMaterial.projectionInverse);
            float viewZ = viewSpace.z * rcp(viewSpace.w);
            
            difference.x += viewZ * kPrewittHorizontalKernel[x][y];
            difference.y += viewZ * kPrewittVerticalKernel[x][y];
        }
    }
    
    PSOutput o;
    float weight = length(difference);
    weight = saturate(weight);
    o.color.rgb = (1.0f - weight) * gTexture.Sample(gSampler, input.texcoord).rgb;
    o.color.a = 1.0f;
    return o;
}
