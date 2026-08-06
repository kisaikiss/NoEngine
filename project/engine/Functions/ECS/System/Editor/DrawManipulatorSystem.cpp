#include "stdafx.h"
#include "DrawManipulatorSystem.h"
#include "engine/Editor/EditTag.h"
#include "engine/Editor/EditUtils.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Functions/Command/EditCommand/ChangeValueCommand.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Runtime/GraphicsCore.h"

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
		ImGuizmo::SetRect(rect.x, rect.y, rect.z, rect.w);
		ImGuizmo::Enable(true);
		Manipulate3D(registry, rect);
		Manipulate2D(registry, rect);

		});

#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}

void DrawManipulatorSystem::Manipulate3D(Registry& registry, const Math::Vector4& sceneRect) {

#ifdef USE_IMGUI
	ImGuizmo::SetOrthographic(false);
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
		float imgX = sceneRect.x;
		float imgY = sceneRect.y;

		constexpr float margin = 8.0f;
		constexpr float buttonSizeY = 28.f;

		// 左上に配置する
		ImVec2 toolbarPos = ImVec2(imgX + margin, imgY + margin);

		// SetCursorScreenPos はウィンドウ内の次のウィジェット位置を移動する
		ImGui::SetCursorScreenPos(toolbarPos);
		// 横並びでボタンを置く
		static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
		if (ImGui::Button("T")) {
			currentOp = ImGuizmo::TRANSLATE;
		}
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("R")) {
			currentOp = ImGuizmo::ROTATE;
		}
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("S")) {
			currentOp = ImGuizmo::SCALE;
		}

		auto* t = registry.GetComponent<TransformComponent>(e);
		Math::Matrix4x4 m = t->MakeAffineMatrix4x4(registry);

		ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentOp, ImGuizmo::WORLD, *(m.m));

		// ワールド行列 → ローカル行列へ変換
		Math::Matrix4x4 localMatrix = m;
		if (t->parent != ECS::INVALID_ENTITY) { // 親のワールド行列を取得する何らかの手段が必要
			if (auto* parentTransform = registry.GetComponent<TransformComponent>(t->parent)) {
				Math::Matrix4x4 parentInverseWorld = parentTransform->MakeAffineMatrix4x4(registry);
				parentInverseWorld.Inverse();
				localMatrix = m * parentInverseWorld;

			}
		}

		float translate[3];
		float rotation[3];
		float scale[3];
		ImGuizmo::DecomposeMatrixToComponents(*(localMatrix.m), translate, rotation, scale);

		static Math::Vector3 oldVector;
		switch (currentOp) {
		case ImGuizmo::TRANSLATE: {
			if (ImGuizmo::IsUsing()) {
				t->translate = localMatrix.GetTranslate();
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

#else
static_cast<void>(registry);
static_cast<void>(sceneRect);
#endif // USE_IMGUI
}
void DrawManipulatorSystem::Manipulate2D(Registry& registry, const Math::Vector4& sceneRect) {

#ifdef USE_IMGUI
	ImGuizmo::SetOrthographic(true);
	auto cameraView = registry.View<DebugCamera2DComponent, Camera2DComponent, Transform2DComponent>();

	Math::Matrix4x4 worldViewMatrix = Math::Matrix4x4::IDENTITY;
	auto size = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	Math::Matrix4x4 worldProjection = MathCalculations::MakeOrthographicMatrix(
		0.f, 0.f, static_cast<float>(size.clientWidth), static_cast<float>(size.clientHeight), 0.1f, 100.f);
	bool existCamera = false;
	Math::Vector2 cameraSize = Math::Vector2::UNIT_SCALE;
	for (auto e : cameraView) {
		auto* camera = registry.GetComponent<Camera2DComponent>(e);
		auto* transform = registry.GetComponent<Transform2DComponent>(e);
		worldViewMatrix = transform->MakeAffineMatrix4x4(registry);
		cameraSize = transform->scale;
		size.clientHeight = static_cast<uint32_t>(camera->height);
		size.clientWidth = static_cast<uint32_t>(camera->width);

		worldProjection = camera->projection;
		existCamera = true;
	}
	worldViewMatrix.Inverse();

	// Screen空間UI用: SpritePassのUI描画と同じ、常に現ウィンドウサイズ基準の正射影を使う
	auto windowSize = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	Math::Matrix4x4 screenViewMatrix = Math::Matrix4x4::IDENTITY;
	Math::Matrix4x4 screenProjection = MathCalculations::MakeOrthographicMatrix(
		0.f, 0.f, static_cast<float>(windowSize.clientWidth), static_cast<float>(windowSize.clientHeight), 0.1f, 100.f);

	isActivePreFrame_ = isActive_;
	auto view = registry.View<Transform2DComponent, EditTag, EditSelectedTag>();
	for (auto e : view) {
		float imgX = sceneRect.x;
		float imgY = sceneRect.y;

		constexpr float margin = 8.0f;
		constexpr float buttonSizeY = 28.f;

		ImVec2 toolbarPos = ImVec2(imgX + margin, imgY + margin);
		ImGui::SetCursorScreenPos(toolbarPos);
		static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
		if (ImGui::Button("T")) { currentOp = ImGuizmo::TRANSLATE; }
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("R")) { currentOp = ImGuizmo::ROTATE; }
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("S")) { currentOp = ImGuizmo::SCALE; }

		auto* t = registry.GetComponent<Transform2DComponent>(e);
		Math::Matrix4x4 m = t->MakeAffineMatrix4x4(registry);

		// このエンティティがScreen空間UIかどうかを判定
		auto* sprite = registry.GetComponent<SpriteComponent>(e);
		bool isScreenSpace = sprite && sprite->space == SpriteSpace::Screen;

		Math::Vector2 anchorOffset = { 0.f, 0.f };
		if (isScreenSpace) {
			// SpritePassの描画時と同じアンカー補正を適用
			anchorOffset.x = sprite->anchor.x * static_cast<float>(windowSize.clientWidth);
			anchorOffset.y = sprite->anchor.y * static_cast<float>(windowSize.clientHeight);
			m.m[3][0] += anchorOffset.x;
			m.m[3][1] += anchorOffset.y;
		} else if (existCamera) {
			// デバッグカメラが存在する場合の画面中央基準補正（ワールド空間のみ）
			m.m[3][0] += static_cast<float>(size.clientWidth) / 2.f * cameraSize.x;
			m.m[3][1] += static_cast<float>(size.clientHeight) / 2.f * cameraSize.y;
		}

		const Math::Matrix4x4& viewMatrix = isScreenSpace ? screenViewMatrix : worldViewMatrix;
		const Math::Matrix4x4& projection = isScreenSpace ? screenProjection : worldProjection;

		ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentOp, ImGuizmo::WORLD, *(m.m));

		// 補正を元に戻す
		if (isScreenSpace) {
			m.m[3][0] -= anchorOffset.x;
			m.m[3][1] -= anchorOffset.y;
		} else if (existCamera) {
			m.m[3][0] -= static_cast<float>(size.clientWidth) / 2.f * cameraSize.x;
			m.m[3][1] -= static_cast<float>(size.clientHeight) / 2.f * cameraSize.y;
		}

		// ワールド行列 → ローカル行列へ変換
		Math::Matrix4x4 localMatrix = m;
		if (t->parent != ECS::INVALID_ENTITY) { // 親のワールド行列を取得する何らかの手段が必要
			if (auto* parentTransform = registry.GetComponent<Transform2DComponent>(t->parent)) {
				Math::Matrix4x4 parentInverseWorld = parentTransform->MakeAffineMatrix4x4(registry);
				parentInverseWorld.Inverse();
				localMatrix = m * parentInverseWorld;

			}
		}

		float translate[3];
		float rotation[3];
		float scale[3];
		ImGuizmo::DecomposeMatrixToComponents(*(localMatrix.m), translate, rotation, scale);

		static Math::Vector2 oldVector;
		switch (currentOp) {
		case ImGuizmo::TRANSLATE: {
			if (ImGuizmo::IsUsing()) {
				t->translate.x = localMatrix.GetTranslate().x;
				t->translate.y = localMatrix.GetTranslate().y;
				isActive_ = true;
			} else {
				isActive_ = false;
			}

			if (isActive_ && !isActivePreFrame_) {
				oldVector = t->translate;
			}

			if (!isActive_ && isActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector2>>(&t->translate, oldVector, t->translate)
				);
			}
			break;
		}
		case ImGuizmo::ROTATE: {
			static float oldRotate;
			if (ImGuizmo::IsUsing()) {
				constexpr float kDegreeToRadian = PI / 180.0f;
				t->rotation = rotation[2] * kDegreeToRadian;
				isActive_ = true;
			} else {
				isActive_ = false;
			}

			if (isActive_ && !isActivePreFrame_) {
				oldRotate = t->rotation;
			}

			if (!isActive_ && isActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<float>>(&t->rotation, oldRotate, t->rotation)
				);
			}
			break;
		}
		case ImGuizmo::SCALE: {
			if (ImGuizmo::IsUsing()) {
				t->scale.x = scale[0];
				t->scale.y = scale[1];
				isActive_ = true;
			} else {
				isActive_ = false;
			}

			if (isActive_ && !isActivePreFrame_) {
				oldVector = t->scale;
			}

			if (!isActive_ && isActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector2>>(&t->scale, oldVector, t->scale)
				);
			}
			break;
		}
		}
	}

#else
	static_cast<void>(registry);
	static_cast<void>(sceneRect);
#endif // USE_IMGUI
}
}
}