struct ShadowPayload
{
    bool occluded;
};

struct ShadowAttributes
{
    float2 bary;
};

[shader("closesthit")]
void ClosestHit_Shadow(inout ShadowPayload payload, in ShadowAttributes attribs)
{
    payload.occluded = true; // 当たった → 影
}
