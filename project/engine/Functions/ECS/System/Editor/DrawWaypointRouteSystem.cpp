#include "stdafx.h"
#include "DrawWaypointRouteSystem.h"
#include "engine/Editor/EditTag.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"
#include "engine/Math/MathInclude.h"
#include "engine/Functions/Renderer/Primitive.h"

namespace NoEngine {
namespace ECS {

using namespace Editor;
using namespace Component;

namespace {

// DrawManipulatorSystem::GetParentWorld3Dと同じロジック。
// TransformRoutineSystemはkeyframeの値をt->translate等へそのまま書き込むため、
// keyframeはエンティティ自身の現在のワールド行列ではなく、t->translateと同じ空間＝
// 親のワールド行列（親がなければ単位行列）を基準にした値として扱う必要がある。
Math::Matrix4x4 GetParentWorld3D(Registry& registry, TransformComponent* t) {
	if (t->parent != ECS::INVALID_ENTITY) {
		if (auto* parentTransform = registry.GetComponent<TransformComponent>(t->parent)) {
			return parentTransform->MakeAffineMatrix4x4(registry);
		}
	}
	return Math::Matrix4x4::IDENTITY;
}

// ローカル(親基準)の点をparentWorldでワールド空間の点へ変換する。
// 行ベクトル・行優先(row-major)前提。DrawManipulatorSystem::WorldToScreenと同じ規約。
// 点の位置だけが欲しいので、並進成分を求める計算のみ行っている
// (kf.MakeAffineMatrix4x4() * parentWorld からGetTranslate()するのと同じ結果になる)。
Math::Vector3 TransformPointByParent(const Math::Vector3& localPoint, const Math::Matrix4x4& parentWorld) {
	const auto& m = parentWorld.m;
	return Math::Vector3(
		localPoint.x * m[0][0] + localPoint.y * m[1][0] + localPoint.z * m[2][0] + m[3][0],
		localPoint.x * m[0][1] + localPoint.y * m[1][1] + localPoint.z * m[2][1] + m[3][1],
		localPoint.x * m[0][2] + localPoint.y * m[1][2] + localPoint.z * m[2][2] + m[3][2]
	);
}

// CatmullRom/Bezierを何本の直線に分割して近似するか。増やすほど滑らかだが線が増える。
constexpr int kCurveSegmentCount = 16;

void DrawCurveSegment(const Math::Vector3& localFrom, const Math::Vector3& localTo,
	const Math::Vector3& localPrev, const Math::Vector3& localNext2,
	InterpolationType interpolation, const Math::Matrix4x4& parentWorld, const Math::Color& color) {

	if (interpolation == InterpolationType::CatmullRom || interpolation == InterpolationType::Bezier) {
		Math::Vector3 ctrl1, ctrl2;
		if (interpolation == InterpolationType::Bezier) {
			MakeBezierControlPoints(localPrev, localFrom, localTo, localNext2, ctrl1, ctrl2);
		}

		Math::Vector3 prevWorldPoint = TransformPointByParent(localFrom, parentWorld);
		for (int s = 1; s <= kCurveSegmentCount; ++s) {
			float sampleT = static_cast<float>(s) / static_cast<float>(kCurveSegmentCount);

			Math::Vector3 localPoint = (interpolation == InterpolationType::CatmullRom)
				? CatmullRomVec3(localPrev, localFrom, localTo, localNext2, sampleT)
				: CubicBezierVec3(localFrom, ctrl1, ctrl2, localTo, sampleT);

			Math::Vector3 worldPoint = TransformPointByParent(localPoint, parentWorld);
			DebugPrimitive::DrawLine(prevWorldPoint, worldPoint, color);
			prevWorldPoint = worldPoint;
		}
		return;
	}

	// Linear/Stepは直線1本で十分
	Math::Vector3 worldFrom = TransformPointByParent(localFrom, parentWorld);
	Math::Vector3 worldTo = TransformPointByParent(localTo, parentWorld);
	DebugPrimitive::DrawLine(worldFrom, worldTo, color);
}

} // namespace

void DrawWaypointRouteSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	const Math::Color kLineColor{ 0.2f, 0.9f, 1.0f, 1.0f };

	auto view = registry.View<TransformRoutineComponent, TransformComponent, EditTag, EditSelectedTag>();
	for (auto e : view) {
		auto* routine = registry.GetComponent<TransformRoutineComponent>(e);
		auto* t = registry.GetComponent<TransformComponent>(e);
		if (routine->keyframes.size() < 2) {
			continue;
		}

		Math::Matrix4x4 parentWorld = GetParentWorld3D(registry, t);

		const int count = static_cast<int>(routine->keyframes.size());
		// loopがtrueなら最後のwaypointから最初のwaypointへも線を引いて輪を閉じる
		const int segments = routine->loop ? count : count - 1;

		for (int i = 0; i < segments; ++i) {
			const int nextIndex = (i + 1) % count;
			const auto& from = routine->keyframes[i];
			const auto& to = routine->keyframes[nextIndex];
			const auto& prev = routine->keyframes[GetRoutinePrevIndex(i, count, routine->loop)];
			const auto& next2 = routine->keyframes[GetRoutineNext2Index(nextIndex, count, routine->loop)];

			DrawCurveSegment(from.translate, to.translate, prev.translate, next2.translate,
				routine->interpolation, parentWorld, kLineColor);
		}
	}
}

}
}