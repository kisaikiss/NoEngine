struct ShadowPayload
{
    bool occluded;
};

[shader("miss")]
void Miss_Shadow(inout ShadowPayload payload)
{
    payload.occluded = false; // 何も当たらなかった → 非影
}
