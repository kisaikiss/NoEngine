cbuffer gCamera : register(b1)
{
    float4x4 ViewProj;
};

struct VSInput
{
    float4 position : POSITION; // ワールド位置.xyz
    float4 color : COLOR; // 頂点カラーを受け取る
    float age : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float age : TEXCOORD0;
    float4 color : COLOR; // ピクセルシェーダへ渡す頂点色
};

VSOutput main(VSInput IN)
{
    VSOutput OUT;
    OUT.position = mul(IN.position, ViewProj);
    OUT.age = IN.age;
    OUT.color = IN.color;
    return OUT;
}