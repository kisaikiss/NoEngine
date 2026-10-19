cbuffer BackgroundConst : register(b1)
{
    float2 iResolution;
    float iTime;
    float _pad;
    float4 iColor;
    float4 iParam;
    float4 iCameraPosition;
    float4 iCameraRight;
    float4 iCameraUp;
    float4 iCameraForward;
    float4 iCameraParams; // x: vertical FOV (radians), y: aspect ratio
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

void mainImage(out float4 fragColor, in float2 fragCoord); // ユーザーのシェーダーが定義する

float4 main(PSInput i) : SV_TARGET
{
    // Y反転
    float2 fragCoord = float2(i.uv.x, 1.0f - i.uv.y) * iResolution;
    float4 c;
    mainImage(c, fragCoord);
    c.rgb *= iColor.rgb;
    c.a = 1.0f;
    return c;
}
