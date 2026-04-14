struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 tedcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PSOutput main(VSOutput input)
{
    PSOutput o;
    o.color = gTexture.Sample(gSampler, input.tedcoord);
	return o;
}