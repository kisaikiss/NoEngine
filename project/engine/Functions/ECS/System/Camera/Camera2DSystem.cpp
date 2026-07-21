#include "stdafx.h"
#include "Camera2DSystem.h"
#include "../../Component/Common/CameraComponent.h"
#include "../../Component/Common/Transform2DComponent.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"

namespace NoEngine {
namespace ECS {
void Camera2DSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	using namespace Component;
	using namespace Math;

	auto cameraView = registry.View<Component::Camera2DComponent, Component::Transform2DComponent>();
	for (auto entity : cameraView) {
		auto* camera = registry.GetComponent<Camera2DComponent>(entity);
		auto* transform = registry.GetComponent<Transform2DComponent>(entity);

		// 画面中央
		Math::Vector2 screenCenter = { camera->width / 2.f, camera->height / 2.f };
		// ビュー行列を作成
		Transform2DComponent t = *transform;
		t.translate -= screenCenter;
		Matrix4x4 view = t.MakeAffineMatrix4x4(registry);
		view.Inverse();

		// 画面中央を基準とするための補正
		Matrix4x4 centerToOrigin;
		centerToOrigin.MakeTranslate(Vector3(-screenCenter.x, -screenCenter.y, 0.f));
		Matrix4x4 originToCenter;
		originToCenter.MakeTranslate(Vector3(screenCenter.x, screenCenter.y, 0.f));

		// ビュー行列の計算
		view = centerToOrigin * view * originToCenter;

		camera->projection = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, camera->width, camera->height, camera->zNear, camera->zFar);
		camera->viewProjection = view * camera->projection;
	}
}
}
}