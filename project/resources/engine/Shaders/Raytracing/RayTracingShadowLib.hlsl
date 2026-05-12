// RayTracingShadowLib.hlsl

struct ShadowPayload
{
    bool occluded;
};

RaytracingAccelerationStructure gSceneTLAS : register(t0);

// Scene SRV テーブル
Texture2D<float4> gWorldPosTex : register(t1);
Texture2D<float4> gNormalTex : register(t3);

// UAV テーブル
RWTexture2D<float> gShadowMask : register(u0);

// Camera
cbuffer CameraCB : register(b0)
{
    float4x4 gInvViewProj;
    float3 gCameraPos;
    float pad0;
    float3 gLightDir;
    float pad1;
};

// Light list
struct LightNums
{
    int directionalLightNum;
    int pointLightNum;
    int spotLightNum;
};
ConstantBuffer<LightNums> gLightNums : register(b1);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};
StructuredBuffer<DirectionalLight> gLights : register(t2);

// ShadowRay の最大距離
static const float gShadowMaxDistance = asfloat(0x7F7FFFFF);

[shader("raygeneration")]
void RayGen_Shadow()
{
    uint2 dispatchIdx = DispatchRaysIndex().xy;
    
   // 深度バッファの代わりに、ワールド座標テクスチャを読み込む
    float4 worldPos = gWorldPosTex.Load(int3(dispatchIdx, 0));

    // 背景（オブジェクトがない場所）のスキップ判定
    if (worldPos.w == 0.0f)
    {
        gShadowMask[dispatchIdx] = 1.0f; // 影なし
        return;
    }
    
    // 法線を読み込む（ワールド空間の法線）
    float3 normal = gNormalTex.Load(int3(dispatchIdx, 0)).xyz;

    // 自己遮蔽を防ぐためのオフセット計算
    // 法線方向に少しだけ浮かせた位置を Ray の起点にする
    const float offsetScale = 0.01f; // メッシュのスケールに合わせて微調整
    float3 offsetPos = worldPos.xyz + (normal * offsetScale);
 
    RayDesc rayDesc;
    rayDesc.Origin = offsetPos;
    rayDesc.Direction = gLightDir;
    rayDesc.TMin = 0.1f;
    rayDesc.TMax = gShadowMaxDistance;

    ShadowPayload payload;
    payload.occluded = false;

    TraceRay(gSceneTLAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, rayDesc, payload);


    gShadowMask[dispatchIdx] = payload.occluded ? 0.0f : 1.0f;
}

[shader("miss")]
void Miss_Shadow(inout ShadowPayload payload)
{
    payload.occluded = false;
}


[shader("closesthit")]
void ClosestHit_Shadow(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.occluded = true;

}