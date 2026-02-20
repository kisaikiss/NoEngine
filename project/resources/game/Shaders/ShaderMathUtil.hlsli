#define PI      3.14159265
#define TWO_PI  6.28318530
#define HALF_PI 1.57079633

float fract(float x)
{
    return x - floor(x);
}

float2 fract(float2 x)
{
    return x - floor(x);
}

float3 fract(float3 x)
{
    return x - floor(x);
}