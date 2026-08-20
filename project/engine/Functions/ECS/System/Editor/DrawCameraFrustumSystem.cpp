#include "stdafx.h"
#include "DrawCameraFrustumSystem.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace ECS {
using namespace Component;

namespace {

// DrawWaypointRouteSystem::GetParentWorld3Dと同じロジック。
// keyframeは親のワールド行列(親がなければ単位行列)を基準にした値として扱う必要がある。
Math::Matrix4x4 GetParentWorld3D(Registry& registry, TransformComponent* t) {
	if (t->parent != ECS::INVALID_ENTITY) {
		if (auto* parentTransform = registry.GetComponent<TransformComponent>(t->parent)) {
			return parentTransform->MakeAffineMatrix4x4(registry);
		}
	}
	return Math::Matrix4x4::IDENTITY;
}

// カメラのローカル空間(FORWARD = +Z前提)での視錐台8頂点をworldMatrixでワールド空間へ変換し、
// Near面/Far面の矩形と、それらを繋ぐ側辺(台形の斜辺に相当)を描画する。
void DrawFrustumLines(const Math::Matrix4x4& worldMatrix, float nearClip, float farClip,
	float fovY, float aspect, const Math::Color& color) {

	float halfNearHeight = nearClip * tanf(fovY * 0.5f);
	float halfNearWidth = halfNearHeight * aspect;
	float halfFarHeight = farClip * tanf(fovY * 0.5f);
	float halfFarWidth = halfFarHeight * aspect;

	// ローカル(カメラ)空間での8頂点。順番は 左下, 右下, 右上, 左上
	Math::Vector3 localCorners[8] = {
		{ -halfNearWidth, -halfNearHeight, nearClip },
		{  halfNearWidth, -halfNearHeight, nearClip },
		{  halfNearWidth,  halfNearHeight, nearClip },
		{ -halfNearWidth,  halfNearHeight, nearClip },

		{ -halfFarWidth, -halfFarHeight, farClip },
		{  halfFarWidth, -halfFarHeight, farClip },
		{  halfFarWidth,  halfFarHeight, farClip },
		{ -halfFarWidth,  halfFarHeight, farClip },
	};

	Math::Matrix4x4 mutableWorld = worldMatrix; // Transform()が非constメンバのため
	Math::Vector3 worldCorners[8];
	for (int i = 0; i < 8; ++i) {
		worldCorners[i] = mutableWorld.Transform(localCorners[i]);
	}

	// Near面の4辺
	for (int i = 0; i < 4; ++i) {
		DebugPrimitive::DrawLine(worldCorners[i], worldCorners[(i + 1) % 4], color);
	}
	// Far面の4辺
	for (int i = 0; i < 4; ++i) {
		DebugPrimitive::DrawLine(worldCorners[4 + i], worldCorners[4 + (i + 1) % 4], color);
	}
	// Near-Farを繋ぐ4辺(視錐台の側面=台形の斜辺)
	for (int i = 0; i < 4; ++i) {
		DebugPrimitive::DrawLine(worldCorners[i], worldCorners[4 + i], color);
	}
}

} // namespace

void DrawCameraFrustumSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	const Math::Color kFrustumColor{ 1.0f, 1.0f, 0.0f, 1.0f };       // 現在のカメラ位置
	const Math::Color kKeyframeFrustumColor{ 1.0f, 0.5f, 0.0f, 0.6f }; // keyframe位置(区別のため半透明のオレンジ)

	auto view = registry.View<CameraComponent, TransformComponent, Editor::EditSelectedTag>();
	for (auto e : view) {
		auto* camera = registry.GetComponent<CameraComponent>(e);
		auto* transform = registry.GetComponent<TransformComponent>(e);

		// 現在のTransformでの視錐台
		Math::Matrix4x4 worldMatrix = transform->MakeAffineMatrix4x4(registry);
		DrawFrustumLines(worldMatrix, camera->nearClip, camera->farClip, camera->fov, camera->aspect, kFrustumColor);

		// TransformRoutineComponentを持つ場合、各keyframeでも視錐台を描画する
		if (auto* routine = registry.GetComponent<TransformRoutineComponent>(e)) {
			if (routine->keyframes.empty()) continue;

			Math::Matrix4x4 parentWorld = GetParentWorld3D(registry, transform);

			for (const auto& kf : routine->keyframes) {
				// TransformKeyframeはTransformを継承しているのでMakeAffineMatrix4x4()がそのまま使える
				Math::Matrix4x4 kfWorld = kf.MakeAffineMatrix4x4() * parentWorld;
				DrawFrustumLines(kfWorld, camera->nearClip, camera->farClip, camera->fov, camera->aspect, kKeyframeFrustumColor);
			}
		}
	}
}

}
}