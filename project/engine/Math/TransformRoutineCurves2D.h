#pragma once
#include "Types/Vector2.h"

// TransformRoutineSystem2D（実際の再生）とDrawWaypointRouteSystem2D（エディタ上のライン描画）の
// 両方が同じ補間結果を参照する必要があるため、カーブ計算をここに切り出している。
// Vector3版はTransformRoutineCurves.hを参照（GetRoutinePrevIndex/GetRoutineNext2Indexはそちらを共用する）。

namespace NoEngine {

// 一様Catmull-Romスプライン。p1→p2の区間をt(0〜1)で補間し、前後のp0/p3で接線の形を決める。
inline Math::Vector2 CatmullRomVec2(const Math::Vector2& p0, const Math::Vector2& p1,
	const Math::Vector2& p2, const Math::Vector2& p3, float t) {
	const float t2 = t * t;
	const float t3 = t2 * t;

	Math::Vector2 result =
		p1 * 2.0f +
		(p2 - p0) * t +
		((p0 * 2.0f + p2 * 4.0f) - (p1 * 5.0f + p3)) * t2 +
		((p1 * 3.0f + p3) - (p0 + p2 * 3.0f)) * t3;

	return result * 0.5f;
}

// 3次ベジエ曲線。p0/p1が始点・終点、c1/c2が制御点。
inline Math::Vector2 CubicBezierVec2(const Math::Vector2& p0, const Math::Vector2& c1,
	const Math::Vector2& c2, const Math::Vector2& p1, float t) {
	const float u = 1.0f - t;

	return p0 * (u * u * u)
		+ c1 * (3.0f * u * u * t)
		+ c2 * (3.0f * u * t * t)
		+ p1 * (t * t * t);
}

// Cardinal Spline -> Bezier変換で制御点を自動生成する
inline void MakeBezierControlPoints2D(const Math::Vector2& prev, const Math::Vector2& from,
	const Math::Vector2& to, const Math::Vector2& next2, Math::Vector2& outCtrl1, Math::Vector2& outCtrl2) {
	constexpr float kTangentScale = 1.0f / 6.0f;
	outCtrl1 = from + (to - prev) * kTangentScale;
	outCtrl2 = to - (next2 - from) * kTangentScale;
}

} // namespace NoEngine