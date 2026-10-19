#include "BackgroundCommon.hlsli"

static float gTime = 0.;
static const float REPEAT = 5.0;

float2 rot2(float2 p, float a)
{
    float c = cos(a);
    float s = sin(a);
    return float2(p.x * c + p.y * s, -p.x * s + p.y * c);
}


float sdBox(float3 p, float3 b)
{
    float3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}


float3 mod(float3 x, float y)
{
    return x - y * floor(x / y);
}

float box(float3 pos, float scale)
{
    pos *= scale;
    float base = sdBox(pos, float3(0.4, 0.4, 0.1)) / 1.5;
    pos.xy *= 5.;
    pos.y -= 3.5;
    pos.xy = rot2(pos.xy, 0.75);
    float result = -base;
    return result;
}

float box_set(float3 pos, float iTime)
{
    float3 pos_origin = pos;
    pos = pos_origin;
    pos.y += sin(gTime * 0.4) * 2.5;
    pos.xy = rot2(pos.xy, 0.8);
    float box1 = box(pos, 2. - abs(sin(gTime * 0.4)) * 1.5);
    pos = pos_origin;
    pos.y -= sin(gTime * 0.4) * 2.5;
    pos.xy = rot2(pos.xy, 0.8);
    float box2 = box(pos, 2. - abs(sin(gTime * 0.4)) * 1.5);
    pos = pos_origin;
    pos.x += sin(gTime * 0.4) * 2.5;
    pos.xy = rot2(pos.xy, 0.8);
    float box3 = box(pos, 2. - abs(sin(gTime * 0.4)) * 1.5);
    pos = pos_origin;
    pos.x -= sin(gTime * 0.4) * 2.5;
    pos.xy = rot2(pos.xy, 0.8);
    float box4 = box(pos, 2. - abs(sin(gTime * 0.4)) * 1.5);
    pos = pos_origin;
    pos.xy = rot2(pos.xy, 0.8);
    float box5 = box(pos, .5) * 6.;
    pos = pos_origin;
    float box6 = box(pos, .5) * 6.;
    float result = max(max(max(max(max(box1, box2), box3), box4), box5), box6);
    return result;
}

float map(float3 pos, float iTime)
{
    float3 pos_origin = pos;
    float box_set1 = box_set(pos, iTime);

    return box_set1;
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 uv = (fragCoord.xy * 2.0f - iResolution.xy) / min(iResolution.x, iResolution.y);
    float3 ro = float3(0.0f, -0.2f, iTime * 4.0f);
    float3 ray = normalize(float3(uv.xy, 1.5f));
    ray.xy = rot2(ray.xy, sin(iTime * 0.03) * 5.0);
    ray.yz = rot2(ray.yz, sin(iTime * 0.05) * 0.2);
    float t = 0.1f;
    float3 col = 0.0f;
    float ac = 0.0f;
    
    [loop]
    for (int i = 0; i < 99; i++)
    {
        float3 pos = ro + ray * t;
        pos = mod(pos - 2., 4.) - 2.;
        float gTime = iTime - float(i) * 0.01;
		
        float d = map(pos, iTime);

        d = max(abs(d), 0.01);
        ac += exp(-d * 23.);

        t += d * 0.55;
    }

    col = ac * 0.02;

    col += float3(0., 0.2 * abs(sin(iTime)), 0.5 + sin(iTime) * 0.2);


    fragColor = float4(col, 1.0 - t * (0.02 + 0.02 * sin(iTime)));
   
}