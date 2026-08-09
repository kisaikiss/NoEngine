#pragma once
#include "Types/Vector3.h"

// TransformRoutineSystem（実際の再生）とDrawWaypointRouteSystem（エディタ上のライン描画）の
// 両方が同じ補間結果を参照する必要があるため、カーブ計算をここに切り出している。
// どちらか片方だけ直しても表示と実際の動きがズレるので注意。

namespace NoEngine {

// 一様Catmull-Romスプライン。p1→p2の区間をt(0〜1)で補間し、前後のp0/p3で接線の形を決める。
inline Math::Vector3 CatmullRomVec3(const Math::Vector3& p0, const Math::Vector3& p1,
	const Math::Vector3& p2, const Math::Vector3& p3, float t) {
	const float t2 = t * t;
	const float t3 = t2 * t;

	Math::Vector3 result =
		p1 * 2.0f +
		(p2 - p0) * t +
		((p0 * 2.0f + p2 * 4.0f) - (p1 * 5.0f + p3)) * t2 +
		((p1 * 3.0f + p3) - (p0 + p2 * 3.0f)) * t3;

	return result * 0.5f;
}

// 3次ベジエ曲線。p0/p1が始点・終点、c1/c2が制御点。
inline Math::Vector3 CubicBezierVec3(const Math::Vector3& p0, const Math::Vector3& c1,
	const Math::Vector3& c2, const Math::Vector3& p1, float t) {
	const float u = 1.0f - t;

	return p0 * (u * u * u)
		+ c1 * (3.0f * u * u * t)
		+ c2 * (3.0f * u * t * t)
		+ p1 * (t * t * t);
}

// Bezierの制御点を、TransformKeyframeに手付けのcontrolIn/controlOutを持たせる代わりに、
// Catmull-Rom相当の接線から自動生成する(Cardinal Spline -> Bezier変換)。
inline void MakeBezierControlPoints(const Math::Vector3& prev, const Math::Vector3& from,
	const Math::Vector3& to, const Math::Vector3& next2, Math::Vector3& outCtrl1, Math::Vector3& outCtrl2) {
	constexpr float kTangentScale = 1.0f / 6.0f;
	outCtrl1 = from + (to - prev) * kTangentScale;
	outCtrl2 = to - (next2 - from) * kTangentScale;
}

// CatmullRom/Bezierが参照する「前」「次の次」のインデックス。
// ループしていないルートの端では、外側のwaypointが存在しないので同じ点を複製し、
// 端点で接線が暴れない（オーバーシュートしない）ようにする。
inline int GetRoutinePrevIndex(int currentIndex, int count, bool loop) {
	int prevIndex = currentIndex - 1;
	if (prevIndex < 0) {
		return loop ? count - 1 : currentIndex;
	}
	return prevIndex;
}

inline int GetRoutineNext2Index(int nextIndex, int count, bool loop) {
	int next2Index = nextIndex + 1;
	if (next2Index >= count) {
		return loop ? 0 : nextIndex;
	}
	return next2Index;
}

} // namespace NoEngine