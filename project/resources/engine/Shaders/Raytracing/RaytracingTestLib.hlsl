RaytracingAccelerationStructure sceneAS : register(t0);
RWTexture2D<float4> output : register(u0);

// Camera
cbuffer CameraCB : register(b0)
{
    float4x4 gInvViewProj;
    float3 gCameraPos;
    float pad0;
};

struct Payload
{
    float4 color;
};

[shader("raygeneration")]
void RayGen()
{
    uint2 dispatchIndex = DispatchRaysIndex().xy;
    uint2 targetSize = DispatchRaysDimensions().xy;

   // 1. スクリーン座標（NDC: -1.0 ～ 1.0）の計算
    float2 uv = (float2(dispatchIndex) / float2(targetSize)) * 2.0f - 1.0f;
    uv.y = -uv.y; // DirectXではY軸の向きが反転するため

    // 2. NDC空間でのターゲット座標を定義 (Z = 1.0 は奥側のクリップ面)
    float4 ndcTarget = float4(uv.x, uv.y, 1.0f, 1.0f);

    // 3. 逆ビュープロジェクション行列を掛けて、ワールド座標系の位置に変換
    float4 worldTarget = mul(gInvViewProj, ndcTarget);
    
    // 4. パースペクティブ除算（wで割ることで正しい3D座標にする）
    worldTarget.xyz /= worldTarget.w;

    RayDesc ray;
    ray.Origin = gCameraPos; // カメラのワールド座標
    
    // 5. カメラ位置からターゲット位置へのベクトルを求め、正規化してレイの方向とする
    ray.Direction = normalize(worldTarget.xyz - gCameraPos);
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