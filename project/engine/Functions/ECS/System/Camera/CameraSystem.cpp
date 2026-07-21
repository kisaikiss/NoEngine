#include "CameraSystem.h"
#include "../../Component/Common/CameraComponent.h"
#include "../../Component/Common/TransformComponent.h"

namespace NoEngine{
namespace ECS {
void CameraSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	auto view = registry.View<Component::CameraComponent>();
	for (auto entity : view) {
		auto* camera = registry.GetComponent<Component::CameraComponent>(entity);
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);

		camera->forGPU.fov = camera->fov;
		camera->entity = entity;
		camera->view = transform->MakeAffineMatrix4x4(registry);
		camera->view.Inverse();
		camera->projection.MakePerspectiveFov(camera->forGPU.fov, camera->aspect, camera->nearClip, camera->farClip);
		camera->forGPU.viewProjection = camera->view * camera->projection;
		camera->forGPU.worldPosition = transform->GetWorldPosition(registry);
	}
}
}
}