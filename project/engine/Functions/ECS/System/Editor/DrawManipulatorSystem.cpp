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

		ImVec2 imgMin = ImGui::GetItemRectMin();
		ImVec2 imgMax = ImGui::GetItemRectMax();
		float imgX = imgMin.x;
		float imgY = imgMin.y;
		float imgW = imgMax.x - imgMin.x;

		// ツールバーのサイズとマージン
		const ImVec2 toolbarSize = ImVec2(160, 28);
		const float margin = 8.0f;

		// 右上に配置する例
		ImVec2 toolbarPos = ImVec2(imgX + imgW - toolbarSize.x - margin, imgY + margin);

		// 保存しておく（SetCursorScreenPos はウィンドウ内の次のウィジェット位置を移動する）
		ImGui::SetCursorScreenPos(toolbarPos);

		// 見た目調整
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(40, 40, 40, 200));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(60, 60, 60, 220));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(80, 80, 80, 240));

		// 横並びでボタンを置く
		static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
		if (ImGui::Button("Translate", ImVec2(50, 24))) {
			// Manipulate 中なら確定して Undo コマンドを作る処理をここで呼ぶ
			currentOp = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();
		if (ImGui::Button("Rotate", ImVec2(50, 24))) {
			currentOp = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if (ImGui::Button("Scale", ImVec2(50, 24))) {
			currentOp = ImGuizmo::SCALE;
		}

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

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
			ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentOp, ImGuizmo::WORLD, *(m.m));

			float translate[3];
			float rotation[3];
			float scale[3];
			ImGuizmo::DecomposeMatrixToComponents(*(m.m), translate, rotation, scale);

			static Math::Vector3 oldVector;
			switch (currentOp) {
			case ImGuizmo::TRANSLATE: {
				if (ImGuizmo::IsUsing()) {
					t->translate = m.GetTranslate();
					isActive_ = true;
				} else {
					isActive_ = false;
				}

				if (isActive_ && !isActivePreFrame_) {
					oldVector = t->translate;
				}

				if (!isActive_ && isActivePreFrame_) {
					EditorCommandOperator::AddCommand(
						std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(&t->translate, oldVector, t->translate)
					);
				}
				break;
			}
			case ImGuizmo::ROTATE: {
				static Math::Quaternion oldQuaternion;
				if (ImGuizmo::IsUsing()) {
					constexpr float kDegreeToRadian = PI / 180.0f;
					Math::Matrix4x4 rotateMatrix;
					rotateMatrix.MakeRotate(Math::Vector3(rotation[0] * kDegreeToRadian, rotation[1] * kDegreeToRadian, rotation[2] * kDegreeToRadian));
					t->rotation.FromRotationMatrix(rotateMatrix);
					isActive_ = true;
				} else {
					isActive_ = false;
				}

				if (isActive_ && !isActivePreFrame_) {
					oldQuaternion = t->rotation;
				}

				if (!isActive_ && isActivePreFrame_) {
					EditorCommandOperator::AddCommand(
						std::make_unique<Command::ChangeValueCommand<Math::Quaternion>>(&t->rotation, oldQuaternion, t->rotation)
					);
				}
				break;
			}
			case ImGuizmo::SCALE: {
				if (ImGuizmo::IsUsing()) {
					t->scale.x = scale[0];
					t->scale.y = scale[1];
					t->scale.z = scale[2];
					isActive_ = true;
				} else {
					isActive_ = false;
				}

				if (isActive_ && !isActivePreFrame_) {
					oldVector = t->scale;
				}

				if (!isActive_ && isActivePreFrame_) {
					EditorCommandOperator::AddCommand(
						std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(&t->scale, oldVector, t->scale)
					);
				}
				break;
			}
			}
		}
		});
	
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}
}
}