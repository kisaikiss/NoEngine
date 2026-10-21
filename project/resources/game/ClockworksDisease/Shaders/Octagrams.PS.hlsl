// ============================================================================
//  Octagrams.PS.hlsl
//  GLSL(ShaderToy)の「ボックス繰り返し + 発光レイマーチ」シェーダーの移植。
//  ・レイの原点/方向は cbuffer のカメラ情報(iCameraPosition / Right / Up / Forward / FOV)から作る
//  ・main() は BackgroundCommon.hlsli 側にあるので、ここでは mainImage() を定義する
//  ・iParam.x: 形の回転角(XY平面)の加算値(ラジアン)。0 で元の見た目、π/2 で90度回転
//  ・iParam.y: XZ平面(Y軸まわり)の回転角(ラジアン)。0 で回転なし
// ============================================================================
#include "BackgroundCommon.hlsli"

// ----------------------------------------------------------------------------
// 調整パラメータ
// ----------------------------------------------------------------------------
// カメラ位置 -> パターン空間への倍率。小さいほど「遠くの背景」として動きが緩やかになる
static const float kSpaceScale = 0.25;
// パターン空間上で自動的に流れる速度(元GLSLは z 方向に 4/sec)。止めたいときは 0
static const float3 kFlow = float3(-1.0, 0.0, 0.0);
// 元GLSLのカメラ高さオフセット
static const float3 kOriginBias = float3(0.0, -0.2, 0.0);

// 形の基準回転角(ラジアン)。元GLSLは 0.8。実際の角度は kBaseAngle + iParam.x
static const float kBaseAngle = 0.8;



#define MARCH_STEPS 99

// ----------------------------------------------------------------------------
// ユーティリティ
// ----------------------------------------------------------------------------
static const float REPEAT = 5.0; // 元コードで未使用(参考のため残す)

// GLSL の mod は負数でも常に [0,y) を返す。HLSL の fmod は異なるので自前で定義する
float3 Mod3(float3 x, float y)
{
    return x - y * floor(x / y);
}

// GLSL の `v * rot(a)` (mat2(c,s,-s,c) を右から掛ける)と同じ回転
float2 Rot(float2 v, float a)
{
    float c = cos(a), s = sin(a);
    return float2(c * v.x + s * v.y, -s * v.x + c * v.y);
}

float SdBox(float3 p, float3 b)
{
    float3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float Box(float3 pos, float scale)
{
    pos *= scale;
    float base = SdBox(pos, float3(0.4, 0.4, 0.1)) / 1.5;
    return -base;
}

// gTime はステップごとに少しずつずらす(残像のような効果)
float BoxSet(float3 pos, float gTime, float angle)
{
    float s = sin(gTime * 0.4);
    float scale = 2.0 - abs(s) * 1.5;
    float off = s * 2.5;
    float3 o = pos;

    float3 p;

    p = o;
    p.y += off;
    p.xy = Rot(p.xy, angle);
    float box1 = Box(p, scale);

    p = o;
    p.y -= off;
    p.xy = Rot(p.xy, angle);
    float box2 = Box(p, scale);

    p = o;
    p.x += off;
    p.xy = Rot(p.xy, angle);
    float box3 = Box(p, scale);

    p = o;
    p.x -= off;
    p.xy = Rot(p.xy, angle);
    float box4 = Box(p, scale);

    p = o;
    p.xy = Rot(p.xy, angle);
    float box5 = Box(p, 0.5) * 6.0;

    float box6 = Box(o, 0.5) * 6.0;

    return max(max(max(max(max(box1, box2), box3), box4), box5), box6);
}

// ----------------------------------------------------------------------------
// mainImage
// ----------------------------------------------------------------------------
void mainImage(out float4 fragColor, in float2 fragCoord)
{
    // --- カメラからのレイ ---
    float fov = iCameraParams.x;
    float aspect = iCameraParams.y;
    if (fov <= 0.0)
        fov = 1.0471976; // 60deg フォールバック
    if (aspect <= 0.0)
        aspect = iResolution.x / iResolution.y;

    float tanHalf = tan(0.5 * fov);
    float2 ndc = 2.0 * fragCoord / iResolution - 1.0; // x: 右が+, y: 上が+

    // カメラ空間のレイ(z が前方)
    float3 rayLocal = normalize(float3(ndc.x * tanHalf * aspect, ndc.y * tanHalf, 1.0));

    // ワールド空間へ
    float3 ray = normalize(iCameraRight.xyz * rayLocal.x
                         + iCameraUp.xyz * rayLocal.y
                         + iCameraForward.xyz * rayLocal.z);

    // レイの原点(パターン空間)
    float3 ro = iCameraPosition.xyz * kSpaceScale + kFlow * iTime + kOriginBias;

    // 形の回転角(ラジアン)。iParam.x は元の角度(kBaseAngle)への加算値なので、0 なら元の見た目になる
    float angle = kBaseAngle + iParam.x;

    // XZ平面(Y軸まわり)の回転角(ラジアン)
    float angleXZ = iParam.y;
    ro.xz  = Rot(ro.xz,  angleXZ);
    ray.xz = Rot(ray.xz, angleXZ);

    // --- レイマーチ ---
    float t = 0.1;
    float ac = 0.0;

    [loop]
    for (int i = 0; i < MARCH_STEPS; ++i)
    {
        float3 pos = ro + ray * t;
        pos = Mod3(pos - 2.0, 4.0) - 2.0; // 4x4x4 の繰り返し空間

        float gTime = iTime - float(i) * 0.01;

        float d = BoxSet(pos, gTime, angle);
        d = max(abs(d), 0.01);
        ac += exp(-d * 23.0);

        t += d * 0.55;
    }

    // --- 色 ---
    float3 col = float3(ac * 0.02, ac * 0.02, ac * 0.02);
    col += float3(0.0,
                  0.2 * abs(sin(iTime)),
                  0.5 + sin(iTime) * 0.2);

    // 元GLSLの alpha(奥行きフェード)。BackgroundCommon.hlsli 側で a=1 に上書きされる
    float alpha = 1.0 - t * (0.02 + 0.02 * sin(iTime));

    fragColor = float4(col, alpha);
}
