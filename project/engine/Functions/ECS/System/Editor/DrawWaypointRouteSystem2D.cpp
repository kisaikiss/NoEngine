#include "stdafx.h"
#include "DrawWaypointRouteSystem2D.h"
#include "engine/Editor/EditTag.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent2D.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Math/MathInclude.h"
#include "engine/Math/TransformRoutineCurves2D.h"
#include "engine/Math/Types/Calculations/Matrix3x3Calculations.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace ECS {

using namespace Editor;
using namespace Component;

namespace {

// TransformRoutineSystem2Dはkeyframeの値をt->translate等へそのまま書き込むため、
// keyframeはエンティティ自身の現在のワールド行列ではなく、親のワールド行列
// （親がなければ単位行列）を基準にした値として扱う必要がある。
Math::Matrix3x3 GetParentWorld2D(Registry& registry, Transform2DComponent* t) {
	if (t->parent != ECS::INVALID_ENTITY) {
		if (auto* parentTransform = registry.GetComponent<Transform2DComponent>(t->parent)) {
			return parentTransform->MakeAffineMatrix3x3(registry);
		}
	}
	return Math::Matrix3x3::IDENTITY;
}

// エンティティがScreen空間UI(SpriteComponent::space == Screen)の場合、
// SpritePass/ManipulateRoutineWaypoints2Dと同じアンカー補正量を返す。
// Screen空間ではウィンドウ中心ではなくアンカー位置基準で座標が決まるため、
// これを描画位置に加算しないとアンカーを変更した際に線・マーカーの位置がズレる。
Math::Vector2 GetScreenAnchorOffset(Registry& registry, Entity e) {
	if (auto* sprite = registry.GetComponent<SpriteComponent>(e)) {
		if (sprite->space == SpriteSpace::Screen) {
			Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
			return Math::Vector2(sprite->anchor.x * windowSize.x, sprite->anchor.y * windowSize.y);
		}
	}
	return Math::Vector2::ZERO;
}

// CatmullRom/Bezierを何本の直線に分割して近似するか。増やすほど滑らかだが線が増える。
constexpr int kCurveSegmentCount = 16;

void DrawCurveSegment2D(const Math::Vector2& localFrom, const Math::Vector2& localTo,
	const Math::Vector2& localPrev, const Math::Vector2& localNext2,
	InterpolationType interpolation, const Math::Matrix3x3& parentWorld,
	const Math::Vector2& anchorOffset, const Math::Color& color) {

	if (interpolation == InterpolationType::CatmullRom || interpolation == InterpolationType::Bezier) {
		Math::Vector2 ctrl1, ctrl2;
		if (interpolation == InterpolationType::Bezier) {
			MakeBezierControlPoints2D(localPrev, localFrom, localTo, localNext2, ctrl1, ctrl2);
		}

		Math::Vector2 prevWorldPoint = MathCalculations::TransformPoint(localFrom, parentWorld) + anchorOffset;
		for (int s = 1; s <= kCurveSegmentCount; ++s) {
			float sampleT = static_cast<float>(s) / static_cast<float>(kCurveSegmentCount);

			Math::Vector2 localPoint = (interpolation == InterpolationType::CatmullRom)
				? CatmullRomVec2(localPrev, localFrom, localTo, localNext2, sampleT)
				: CubicBezierVec2(localFrom, ctrl1, ctrl2, localTo, sampleT);

			Math::Vector2 worldPoint = MathCalculations::TransformPoint(localPoint, parentWorld) + anchorOffset;
			DebugPrimitive::DrawLine2D(prevWorldPoint, worldPoint, color);
			prevWorldPoint = worldPoint;
		}
		return;
	}

	// Linear/Stepは直線1本で十分
	Math::Vector2 worldFrom = MathCalculations::TransformPoint(localFrom, parentWorld) + anchorOffset;
	Math::Vector2 worldTo = MathCalculations::TransformPoint(localTo, parentWorld) + anchorOffset;
	DebugPrimitive::DrawLine2D(worldFrom, worldTo, color);
}

} // namespace

void DrawWaypointRouteSystem2D::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	const Math::Color kLineColor{ 0.2f, 0.9f, 1.0f, 1.0f };

	auto view = registry.View<TransformRoutineComponent2D, Transform2DComponent, EditTag, EditSelectedTag>();
	for (auto e : view) {
		auto* routine = registry.GetComponent<TransformRoutineComponent2D>(e);
		auto* t = registry.GetComponent<Transform2DComponent>(e);
		if (routine->keyframes.size() < 2) {
			continue;
		}

		Math::Matrix3x3 parentWorld = GetParentWorld2D(registry, t);
		Math::Vector2 anchorOffset = GetScreenAnchorOffset(registry, e);

		const int count = static_cast<int>(routine->keyframes.size());
		// loopがtrueなら最後のwaypointから最初のwaypointへも線を引いて輪を閉じる
		const int segments = routine->loop ? count : count - 1;

		for (int i = 0; i < segments; ++i) {
			const int nextIndex = (i + 1) % count;
			const auto& from = routine->keyframes[i];
			const auto& to = routine->keyframes[nextIndex];
			const auto& prev = routine->keyframes[GetRoutinePrevIndex(i, count, routine->loop)];
			const auto& next2 = routine->keyframes[GetRoutineNext2Index(nextIndex, count, routine->loop)];

			DrawCurveSegment2D(from.translate, to.translate, prev.translate, next2.translate,
				routine->interpolation, parentWorld, anchorOffset, kLineColor);
		}
		for (auto& kf : routine->keyframes) {
			// Transformの正しい世界座標系での値を取得するためにこの瞬間だけキーフレームに親を設定する
			kf.parent = t->parent;
			Math::Vector2 worldPos = kf.GetWorldPosition(registry) + anchorOffset;
			Math::Vector2 half = kf.GetWorldScale(registry) * 0.5f;
			kf.parent = INVALID_ENTITY;
			DebugPrimitive::DrawCube2D(worldPos, half, -half, Math::Color::RED);
		}
	}
}

}
}