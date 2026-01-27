#include "ShaderMathUtil.hlsli"

cbuffer gWorld : register(b0)
{
    float4x4 WorldMat;
    float4x4 UVTransform;
    float4x4 ViewProjMat;
    float time;
};

float3 drawCircle(float2 pos, float radius, float width, float power, float4 color)
{
    float dist1 = length(pos);
    dist1 = fract((dist1 * 5.0) - fract(time));
    float dist2 = dist1 - radius;
    float intensity = pow(radius / abs(dist2), width);
    float3 col = color.rgb * intensity * power * max((0.8 - abs(dist2)), 0.0);
    return col;
}

float3 hsv2rgb(float h, float s, float v)
{
    float4 t = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(fract(float3(h,h,h) + t.xyz) * 6.0 - float3(t.w,t.w,t.w));
    return v * lerp(float3(t.x, t.x, t.x), clamp(p - float3(t.x, t.x, t.x), 0.0, 1.0), s);
}

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

float4 main(VSOutput input) : SV_TARGET
{
    float2 pos = input.uv * 2.0 - 1.0;;
    
    float h = lerp(0.5, 0.65, length(pos));
    float4 color = float4(hsv2rgb(h, 1.0, 1.0), 1.0);
    float radius = 0.5;
    float width = 0.5;
    float power = 0.1;
    float3 finalColor = drawCircle(pos, radius, width, power, color);
    finalColor = pow(finalColor, float3(0.5, 0.5, 0.5));
    pos = abs(pos);
    
    return float4(finalColor, 1.0);
    
    //float2 uv = input.uv * 2.0 - 1.0;
    //   Raymarch depth
    //float z = 0,
    //Step distance
    //d = 0,
    //Raymarch iterator
    //i = 0;
    //Clear fragColor and raymarch 20 steps
    
    //float4 color = 0;
    //for (color *= i; i++ < 2e1;)
    //{
    //    float3 rayDir = normalize(float3(uv * float2(1.0, 1.0), -1.0));
    //    float3 p = z * rayDir + 0.1;
    //    Polar coordinates and additional transformations
    //    p = float3(atan2(p.y / 0.2, max(abs(p.x), 0.001)) * 2., p.z / 3., length(p.xy) - 5. - z * .2);
        
    //    Apply turbulence and refraction effect
    //    for (d = 0.; d++ < 7.;)
    //        p += sin(p.yzx * d + time + .3 * i) / d;
            
    //    Distance to cylinder and waves with refraction
    //    z += d = length(float4(.4 * cos(p) - .4, p.z));
        
    //    Coloring and brightness
    //    color += (1. + cos(p.x + i * .4 + z + float4(6, 1, 2, 0))) / d;
    //}
    //Tanh tonemap
    //color = tanh(color * color / 4e2);
    //return float4(color.rgb, 1.0);
}