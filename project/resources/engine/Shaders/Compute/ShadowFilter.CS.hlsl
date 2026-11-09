Texture2DArray<float> gShadowMask : register(t0);
Texture2D<float4> gWorldPosition : register(t1);
Texture2D<float4> gNormal : register(t2);
RWTexture2D<float> gFilteredShadow : register(u0);

cbuffer FilterConstants : register(b0)
{
    float4 gViewDepthRow;
    uint gShadowSlice;
};

[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    uint2 size;
    gFilteredShadow.GetDimensions(size.x, size.y);
    if (any(dispatchId.xy >= size))
        return;

    int2 pixel = int2(dispatchId.xy);
    float4 centerPosition = gWorldPosition.Load(int3(pixel, 0));
    float centerShadow = gShadowMask.Load(int4(pixel, gShadowSlice, 0));
    if (centerPosition.w == 0.0f)
    {
        gFilteredShadow[pixel] = centerShadow;
        return;
    }

    // ワールド座標をビュー空間のZに変換し、線形深度の差で重みを落とす。
    float centerDepth = dot(float4(centerPosition.xyz, 1.0f), gViewDepthRow);
    float depthRange = max(0.01f, abs(centerDepth) * 0.02f);
    float3 centerNormal = normalize(gNormal.Load(int3(pixel, 0)).xyz * 2.0f - 1.0f);
    float shadowSum = 0.0f;
    float weightSum = 0.0f;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            int2 samplePixel = pixel + int2(x, y);
            if (any(samplePixel < 0) || any(samplePixel >= int2(size)))
                continue;

            float4 samplePosition = gWorldPosition.Load(int3(samplePixel, 0));
            if (samplePosition.w == 0.0f)
                continue;

            float sampleDepth = dot(float4(samplePosition.xyz, 1.0f), gViewDepthRow);
            float depthWeight = 1.0f - smoothstep(0.0f, depthRange, abs(sampleDepth - centerDepth));
            float3 sampleNormal = normalize(gNormal.Load(int3(samplePixel, 0)).xyz * 2.0f - 1.0f);
            float normalWeight = smoothstep(0.85f, 1.0f, dot(sampleNormal, centerNormal));
            float spatialWeight = (2 - abs(x)) * (2 - abs(y));
            float weight = spatialWeight * depthWeight * normalWeight;
            shadowSum += weight * gShadowMask.Load(int4(samplePixel, gShadowSlice, 0));
            weightSum += weight;
        }
    }

    gFilteredShadow[pixel] = weightSum > 0.0f ? shadowSum / weightSum : centerShadow;
}
