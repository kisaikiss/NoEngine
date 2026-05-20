static const int MAX_KERNEL_SIZE = 49;
static const int MAX_KERNEL_VEC = 13; // (49 + 3) / 4

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer KernelCB : register(b0)
{
    int kernelSize; // 実際のカーネル幅（奇数）
    float3 _pad;
    float4 kernel[MAX_KERNEL_VEC];
};


PSOutput main(VSOutput input)
{
    PSOutput o;
    o.color = float4(0, 0, 0, 1);

    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStep = float2(rcp(float(width)), rcp(float(height)));

    int ks = max(1, kernelSize);
    if (ks > MAX_KERNEL_SIZE)
        ks = MAX_KERNEL_SIZE;

    int halfKs = ks / 2;

    for (int ky = -halfKs; ky <= halfKs; ++ky)
    {
        for (int kx = -halfKs; kx <= halfKs; ++kx)
        {
            int ix = kx + halfKs;
            int iy = ky + halfKs;
            int idx = iy * ks + ix; // 1D インデックス (0〜24 など)

            float2 sampleUV = input.texcoord + float2(kx, ky) * uvStep;
            sampleUV = clamp(sampleUV, float2(0.0f, 0.0f), float2(1.0f, 1.0f));

            float3 sample = gTexture.Sample(gSampler, sampleUV).rgb;
            
            // 1Dインデックスから float4 配列のインデックスと、その中の何番目の要素かを計算する
            int vecIdx = idx / 4;
            int elemIdx = idx % 4;
            float k = kernel[vecIdx][elemIdx];
          
            o.color.rgb += sample * k;
        }
    }

    return o;
}