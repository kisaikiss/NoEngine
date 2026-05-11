RaytracingAccelerationStructure sceneAS : register(t0);
RWTexture2D<float4> output : register(u0); 

struct Payload
{
    float4 color;
};
 
cbuffer CameraCB : register(b0)
{
    float3 gCameraPos;
    float pad0;
    float3 gCameraForward;
    float pad1;
    float3 gCameraRight;
    float pad2;
    float3 gCameraUp;
    float pad3;
};

[shader("raygeneration")]
void RayGen()
{
    uint2 dispatchIndex = DispatchRaysIndex().xy;
    uint2 targetSize = DispatchRaysDimensions().xy;

    // NDC座標系 (-1.0 ～ 1.0)
    float2 uv = (float2(dispatchIndex) / float2(targetSize)) * 2.0f - 1.0f;
    uv.y = -uv.y; // DirectX座標系に合わせてY反転

    RayDesc ray;
    ray.Origin = gCameraPos;
    
    // カメラの正面ベクトルに、UV座標とスケール済み右・上ベクトルを足し合わせて方向を決定
    float3 rayDir = gCameraForward + (uv.x * gCameraRight) + (uv.y * gCameraUp);
    ray.Direction = normalize(rayDir);
    
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    Payload payload;
    payload.color = float4(0.0f, 0.0f, 0.0f, 1.0f);

    TraceRay(sceneAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    output[dispatchIndex] = payload.color;
}

[shader("miss")]
void MissShader(inout Payload payload)
{
    payload.color = float4(0.0f, 0.2f, 0.8f, 1.0f);
}

[shader("closesthit")]
void ClosestHitShader(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float u = attr.barycentrics.x;
    float v = attr.barycentrics.y;
    float w = 1.0f - u - v;
    payload.color = float4(u, v, 0.0f, 1.0f);
}