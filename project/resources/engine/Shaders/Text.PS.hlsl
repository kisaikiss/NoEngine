struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

// SDFフォントアトラス。FontManagerでDXGI_FORMAT_R8_UNORMとしてベイクしているため単一チャンネル(.r)。
Texture2D<float> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer gMaterial : register(b0)
{
    float4 color;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float distance = gTexture.Sample(gSampler, input.texcoord);

    // fwidthでスクリーン空間の変化量を見て平滑化幅を決める。
    // 文字の拡大率やカメラ距離が変わってもエッジのギザギザ/にじみが出にくい。
    float smoothing = max(fwidth(distance), 1e-5f);
    float alpha = smoothstep(0.5f - smoothing, 0.5f + smoothing, distance);

    if (alpha <= 0.001f)
    {
        discard;
    }

    output.color = float4(color.rgb, color.a * alpha);
    return output;
}