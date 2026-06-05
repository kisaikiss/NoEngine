#include "stdafx.h"
#include "DrawManipulatorSystem.h"
#include "engine/Editor/EditTag.h"
#include "engine/Editor/EditUtils.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Functions/Command/EditCommand/ChangeValueCommand.h"

#ifdef USE_IMGUI
#include "externals/imgui/ImGuizmo.h"
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
namespace ECS {

using namespace Editor;
using namespace Component;

void DrawManipulatorSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

#ifdef USE_IMGUI
	SetGizmoCallback([this, &registry](const Math::Vector4& rect) {
		ImGuizmo::SetDrawlist();

		auto cameraView = registry.View<DebugCameraComponent, CameraComponent>();
		Math::Matrix4x4 viewMatrix;
		Math::Matrix4x4 projection;
		for (auto e : cameraView) {
			auto* camera = registry.GetComponent<CameraComponent>(e);
			viewMatrix = camera->view;
			projection = camera->projection;
		}
		isActivePreFrame_ = isActive_;
		auto view = registry.View<TransformComponent, EditTag, EditSelectedTag>();
		for (auto e : view) {
			auto* t = registry.GetComponent<TransformComponent>(e);
			Math::Matrix4x4 m = t->MakeAffineMatrix4x4();
			ImGuizmo::SetRect(rect.x, rect.y, rect.z, rect.w);
			ImGuizmo::Enable(true);
			isActive_ = ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), ImGuizmo::TRANSLATE, ImGuizmo::WORLD, *(m.m));

			static Math::Vector3 oldVector;
			if (isActive_ && !isActivePreFrame_) {
				oldVector = t->translate;
			}

			if (ImGuizmo::IsUsing()) {
				t->translate = m.GetTranslate();
			}

			if (!isActive_ && isActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(&t->translate, oldVector, t->translate)
				);
			}
		}
		});
	
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}
}
}