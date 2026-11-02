#include "Grid.hlsli"

float GridMask(float2 coordinate, float2 footprint, float spacing, float width)
{
    float2 cell = coordinate / spacing;
    float2 pixelSize = max(footprint / spacing, 1e-7);
    float2 distanceToLine = abs(frac(cell + 0.5) - 0.5) / pixelSize;
    float2 coverage = 1 - smoothstep(max(0, width * 0.5 - 0.5), width * 0.5 + 0.5, distanceToLine);
    // サブピクセルに密集した線は方向ごとに消し、モアレを抑える。
    coverage *= smoothstep(2, 8, 1 / pixelSize);
    return max(coverage.x, coverage.y);
}

float4 main(GridVertex input) : SV_TARGET
{
    float2 coordinate = PlaneCoordinates(input.worldPosition - originSpacing.xyz);
    // 繰り返し処理前の連続座標から微分を計算する。
    float2 dx = ddx(coordinate);
    float2 dy = ddy(coordinate);
    float2 footprint = sqrt(dx * dx + dy * dy);
    float ratio = widthsMajor.w;
    float lod = options.y > 0.5
        ? max(0, log(max(max(footprint.x, footprint.y) * 12 / originSpacing.w, 1e-7)) / log(ratio))
        : 0;
    float spacing = originSpacing.w * pow(ratio, floor(lod));
    float blend = smoothstep(0, 1, frac(lod));
    float fine = GridMask(coordinate, footprint, spacing, widthsMajor.x);
    float nextFine = GridMask(coordinate, footprint, spacing * ratio, widthsMajor.x);
    float middle = GridMask(coordinate, footprint, spacing * ratio, widthsMajor.y);
    float coarse = GridMask(coordinate, footprint, spacing * ratio * ratio, widthsMajor.y);
    // 同じ位置の線を加算せず、主線の色へ置き換える。
    // 段階境界でも補助線と主線の両方を連続させる。
    float majorMask = lerp(middle, coarse, blend);
    float minorAlpha = lerp(fine, nextFine, blend) * minorColor.a * (1 - majorMask);
    float majorAlpha = majorMask * majorColor.a;
    float alpha = majorAlpha + minorAlpha;
    float3 premultiplied = majorColor.rgb * majorAlpha + minorColor.rgb * minorAlpha;

    float2 axisDistance = abs(coordinate) / max(footprint, 1e-7);
    float2 axisMask = (1 - smoothstep(max(0, widthsMajor.z * 0.5 - 0.5), widthsMajor.z * 0.5 + 0.5, axisDistance)) * options.z;
    float3 uColor = options.x > 1.5 ? float3(0.15, 0.65, 0.25) : float3(0.85, 0.15, 0.12);
    float3 vColor = options.x > 0.5 && options.x < 1.5 ? float3(0.15, 0.65, 0.25) : float3(0.15, 0.35, 0.95);
    float axisAlpha = max(axisMask.x, axisMask.y) * 0.85;
    float3 axisColor = axisMask.y >= axisMask.x ? uColor : vColor;
    premultiplied = axisColor * axisAlpha + premultiplied * (1 - axisAlpha);
    alpha = axisAlpha + alpha * (1 - axisAlpha);

    float distanceFromCenter = length(PlaneCoordinates(input.worldPosition) - centerRadius.xy);
    float fade = 1 - smoothstep(centerRadius.z * centerRadius.w, centerRadius.z, distanceFromCenter);
    // 遠方と地平線付近を滑らかに消す。平面の裏側からも表示する。
    float3 toCamera = cameraPosition.xyz - input.worldPosition;
    float normalDistance = options.x < 0.5 ? abs(toCamera.y) : (options.x < 1.5 ? abs(toCamera.z) : abs(toCamera.x));
    fade *= smoothstep(0.005, 0.04, normalDistance / max(length(toCamera), 1e-5));
    fade *= 1 - smoothstep(centerRadius.z, centerRadius.z * 2, length(toCamera));
    return float4(premultiplied / max(alpha, 1e-7), alpha * fade);
}
