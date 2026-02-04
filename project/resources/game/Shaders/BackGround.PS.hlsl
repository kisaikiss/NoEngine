#include "ShaderMathUtil.hlsli"

cbuffer gControl : register(b1)
{
    float4 bgColor;
    float4 ringColor;
    float time;
    float timeScale;
    float powerFactor;
    float fadeInner;
    float fadeOuter;
    float seed;
    float variant;
    uint useRing;
}

// noise background
static const float noiseIntensity = 2.8;
static const float noiseDefinition = 0.6;
static const float2 glowPos = float2(-2.0, 0.0);

// rectangles
static const float total = 60.0;
static const float minSize = 0.01;
static const float maxSize = 0.03 - minSize;
static const float yDistribution = 0.5;

float2x2 rotate2d(float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2x2(c, -s, s, c);
}
float2 hash22(float2 p)
{
    p = float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)));
    return -1.0 + 2.0 * frac(sin(p) * 43758.5453123 + seed); // 混入seed
}

float random(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

float noise(float2 p)
{
    p *= noiseIntensity;

    float2 i = floor(p);
    float2 f = frac(p);
    float2 u = f * f * (3.0 - 2.0 * f);

    float a = random(i + float2(0.0, 0.0));
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    return lerp(lerp(a, b, u.x),
                lerp(c, d, u.x), u.y);
}

float fbm(float2 uv)
{
    uv *= 5.0;

    float2x2 m = float2x2(
        1.6, 1.2,
       -1.2, 1.6
    );

    float f = 0.5000 * noise(uv);
    uv = mul(m, uv);
    f += 0.2500 * noise(uv);
    uv = mul(m, uv);
    f += 0.1250 * noise(uv);
    uv = mul(m, uv);
    f += 0.0625 * noise(uv);
    uv = mul(m, uv);

    return 0.5 + 0.5 * f;
}
float3 bg(float2 uv)
{
    float velocity = time / 1.6;
    float intensity = sin(uv.x * 3.0 + velocity * 2.0) * 1.1 + 1.5;

    uv.y -= 2.0;

    float2 bp = uv + glowPos;
    uv *= noiseDefinition;

    // ripple
    float rb = fbm(float2(uv.x * 0.5 - velocity * 0.03, uv.y)) * 0.1;
    uv += rb;

    // coloring
    float rz = fbm(uv * 0.9 + float2(velocity * 0.35, 0.0));
    rz *= dot(bp * intensity, bp) + 1.2;

    float3 col = bgColor.rgb / (0.1 - rz);
    return sqrt(abs(col));
}

float ring(
    float2 uv,
    float2 center,
    float radius,
    float thickness,
    float blur
)
{
    float dist = length(uv - center);

    float inner = radius - thickness * 0.5;
    float outer = radius + thickness * 0.5;

    float ringMask =
        smoothstep(inner - blur, inner + blur, dist) *
        (1.0 - smoothstep(outer - blur, outer + blur, dist));

    return ringMask;
}


float3 drawCircle(float2 pos, float radius, float width, float power, float4 color)
{
    float distortion = noise(pos * 3.0 + float2(time * 0.2, time * 0.2));
    
    float2 noisyPos = pos + distortion * 0.01;

    float dist1 = length(noisyPos);
    
    dist1 = fract((dist1 * 5.0f) - fract(time * timeScale));
    float dist2 = dist1 - radius;
    float intensity = pow(radius / abs(dist2), width);
    float3 col = color.rgb * intensity * power * max((0.8 - abs(dist2)), 0.0);
    return col;
}
//float3 drawCircle(float2 pos, float radius, float width, float power, float4 color)
//{
//    float dist1 = length(pos);
//    dist1 = fract((dist1 * 5.0f) - fract(time * timeScale));
//    float dist2 = dist1 - radius;
//    float intensity = pow(radius / abs(dist2), width);
//    float3 col = color.rgb * intensity * power * max((0.8 - abs(dist2)), 0.0);
//    return col;
//}

float3 hsv2rgb(float h, float s, float v)
{
    float4 t = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(fract(float3(h, h, h) + t.xyz) * 6.0 - float3(t.w, t.w, t.w));
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
    float2 uv = input.uv * 2.0 - 1.0;
    float2 uvCenter = uv;
    float baseH = lerp(0.45, 0.7, length(uv));
    float h = fract(baseH + seed * 0.13);
    
    float3 finalColor = 0.0f;
    if (useRing == 1)
    {
        float4 color = float4(hsv2rgb(h, 0.9, 0.9), 1.0);
        float radius = 0.5;
        float width = 0.75;
        float power = powerFactor;
        finalColor = drawCircle(uv, radius, width, power, color);
        finalColor = pow(finalColor, 0.454545);
        finalColor.rgb *= 0.55;
        uv = abs(uv);
    }
    else
    {
        finalColor = bg(uv) * (2.0 - abs(uv.y * 2.0));
        float velX = -time / 8.0;
        float velY = time / 10.0;

    [loop]
        for (int i = 0; i < (int) total; ++i)
        {
            float index = (float) i / total;
            float rnd = random(float2(index, 0.0));

            float3 pos;
            pos.x = frac(velX * rnd + index) * 4.0 - 2.0;
            pos.y = sin(index * rnd * 1000.0 + velY) * yDistribution;
            pos.z = maxSize * rnd + minSize;

            float2 uvRot = uv - pos.xy + pos.z * 0.5;
            uvRot = mul(rotate2d(i + time * 0.5), uvRot);
            uvRot += pos.xy + pos.z * 0.5;

            float radius = pos.z * 0.5;
            float thickness = pos.z * 0.25;
            float blur = (maxSize + minSize - pos.z) * 0.5;

            float r = ring(
                    uvRot,
                    pos.xy,
                    radius,
                    thickness,
                    blur);

            finalColor += ringColor.rgb * r * (pos.z / maxSize);
        }
    }
    
    //   Raymarch depth
    float z = 0,
    //Step distance
    d = 0,
    //Raymarch iterator
    i = 0;
    //Clear fragColor and raymarch 10 steps
    
    float4 color2 = 0;
    float2x2 bgRot = rotate2d(seed * PI);
    for (color2 *= i; i++ < 10;)
    {
        float3 rayDir = normalize(float3(uv * float2(1.0, 1.0), -1.0));
        rayDir.xy = mul(rayDir.xy, bgRot);
        float3 p = z * rayDir + 0.1;
      //  Polar coordinates and additional transformations
        p = float3(atan2(p.y / 0.2, max(abs(p.x), 0.001)) * 2., p.z / 3., length(p.xy) - 5. - z * .2);
      
        //Apply turbulence and refraction effect
        for (d = 0.; d++ < 7.;)
            p += sin(p.yzx * d + time + seed * 5.0 + .3 * i) / d;
          
        //Distance to cylinder and waves with refraction
        z += d = length(float4(.4 * cos(p) - .4, p.z));
      
        //Coloring and brightness
        color2 += (1. + cos(p.x + i * .4 + z + float4(6, 1, 2, 0))) / d;
    }
    //Tanh tonemap
    color2 = tanh(color2 * color2 / 4e2);
    
    float bgHueOffset = fract(h + 0.5);
    float3 bgTint = hsv2rgb(bgHueOffset, 0.4, 0.5);
    finalColor += color2.rgb * (0.6 + 0.4 * bgTint);
    
    float distCenter = length(uvCenter);
    float fade = 1.0 - smoothstep(fadeInner, fadeOuter, distCenter);
    finalColor.rgb *= fade;
    return float4(finalColor.rgb * 0.3, 1.0);
}