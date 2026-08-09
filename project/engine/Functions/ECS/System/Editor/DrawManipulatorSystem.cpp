#include "stdafx.h"
#include "DrawManipulatorSystem.h"
#include "engine/Editor/EditTag.h"
#include "engine/Editor/EditUtils.h"
#include "engine/Editor/EditorCommandOperator.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"
#include "engine/Functions/Command/EditCommand/ChangeValueCommand.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <string>

#ifdef USE_IMGUI
#include "externals/imgui/ImGuizmo.h"
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
namespace ECS {

namespace {
bool sTriggerButton = false;
}

using namespace Editor;
using namespace Component;

void DrawManipulatorSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

#ifdef USE_IMGUI
	sTriggerButton = false;
	SetGizmoCallback([this, &registry](const Math::Vector4& rect) {
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(rect.x, rect.y, rect.z, rect.w);
		ImGuizmo::Enable(true);
		Manipulate3D(registry, rect);
		Manipulate2D(registry, rect);
		ManipulateRoutineWaypoints(registry, rect);

		});

#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}

bool DrawManipulatorSystem::TriggerManipulateButton() {
	return sTriggerButton;
}

void DrawManipulatorSystem::Manipulate3D(Registry& registry, const Math::Vector4& sceneRect) {

#ifdef USE_IMGUI
	ImGuizmo::SetOrthographic(false);
	Math::Matrix4x4 viewMatrix;
	Math::Matrix4x4 projection;
	GetActiveCamera3D(registry, viewMatrix, projection);

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
			sTriggerButton = true;
		}
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("R")) {
			currentOp = ImGuizmo::ROTATE;
			sTriggerButton = true;
		}
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("S")) {
			currentOp = ImGuizmo::SCALE;
			sTriggerButton = true;
		}

		// TransformRoutineComponentを持つエンティティは、自身のTransform編集と
		// waypoint編集(ManipulateRoutineWaypoints)を切り替えられるようにする
		bool hasRoutine = registry.Has<TransformRoutineComponent>(e);
		if (hasRoutine) {
			toolbarPos.y += buttonSizeY;
			ImGui::SetCursorScreenPos(toolbarPos);
			// Push/Popの対応を崩さないよう、クリック前の状態をローカルに固定してから使う
			// (routineEditMode_をボタン内で書き換えた後の値をPop側で読むとPush/Popが不一致になる)
			bool wasRouteEditMode = routineEditMode_;
			if (wasRouteEditMode) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
			if (ImGui::Button("Route")) {
				routineEditMode_ = !routineEditMode_;
				sTriggerButton = true;
			}
			if (wasRouteEditMode) ImGui::PopStyleColor();
		}

		// waypoint編集モード中は、このエンティティ自体のTransformは動かさない
		// (1フレームに2つのImGuizmo::Manipulateを重ねて呼ぶと競合するため)
		if (hasRoutine && routineEditMode_) {
			continue;
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
		if (ImGui::Button("T")) {
			currentOp = ImGuizmo::TRANSLATE;
			sTriggerButton = true;
		}
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("R")) {
			currentOp = ImGuizmo::ROTATE;
			sTriggerButton = true;
		}
		toolbarPos.y += buttonSizeY;
		ImGui::SetCursorScreenPos(toolbarPos);
		if (ImGui::Button("S")) {
			currentOp = ImGuizmo::SCALE;
			sTriggerButton = true;
		}

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

bool DrawManipulatorSystem::GetActiveCamera3D(Registry& registry, Math::Matrix4x4& outView, Math::Matrix4x4& outProjection) {
	auto cameraView = registry.View<DebugCameraComponent, CameraComponent>();

	Entity entity = INVALID_ENTITY;

	auto it = cameraView.begin();
	if (it != cameraView.end()) {
		entity = *it;
	}

	if (entity != INVALID_ENTITY) {
		auto* camera = registry.GetComponent<CameraComponent>(entity);
		outView = camera->view;
		outProjection = camera->projection;
		return true;
	}
	return false;
}

Math::Matrix4x4 DrawManipulatorSystem::GetParentWorld3D(Registry& registry, Component::TransformComponent* t) {
	if (t->parent != ECS::INVALID_ENTITY) {
		if (auto* parentTransform = registry.GetComponent<TransformComponent>(t->parent)) {
			return parentTransform->MakeAffineMatrix4x4(registry);
		}
	}
	return Math::Matrix4x4::IDENTITY;
}

// worldPos を view*projection でクリップ空間に変換し、sceneRect内のスクリーン座標へ落とし込む。
// MakeAffineMatrix4x4 / t->parent周りの掛け順（m * parentInverseWorld）に合わせた
// 行ベクトル・行優先(row-major)前提の実装。Math::Matrix4x4の実際の規約と食い違いがあれば
// 掛け順・添字を読み替えること。
bool DrawManipulatorSystem::WorldToScreen(const Math::Vector3& worldPos, const Math::Matrix4x4& viewProjection,
	const Math::Vector4& sceneRect, Math::Vector2& outScreenPos) {

	const auto& m = viewProjection.m;
	float x = worldPos.x, y = worldPos.y, z = worldPos.z;

	float clipX = x * m[0][0] + y * m[1][0] + z * m[2][0] + m[3][0];
	float clipY = x * m[0][1] + y * m[1][1] + z * m[2][1] + m[3][1];
	float clipW = x * m[0][3] + y * m[1][3] + z * m[2][3] + m[3][3];

	if (clipW <= 0.0f) {
		return false; // カメラの後ろにある
	}

	float ndcX = clipX / clipW;
	float ndcY = clipY / clipW;

	outScreenPos.x = sceneRect.x + (ndcX * 0.5f + 0.5f) * sceneRect.z;
	outScreenPos.y = sceneRect.y + (1.0f - (ndcY * 0.5f + 0.5f)) * sceneRect.w;
	return true;
}

void DrawManipulatorSystem::ManipulateRoutineWaypoints(Registry& registry, const Math::Vector4& sceneRect) {
#ifdef USE_IMGUI
	if (!routineEditMode_) {
		return;
	}

	ImGuizmo::SetOrthographic(false);
	Math::Matrix4x4 viewMatrix;
	Math::Matrix4x4 projection;
	if (!GetActiveCamera3D(registry, viewMatrix, projection)) {
		return;
	}
	Math::Matrix4x4 viewProjection = viewMatrix * projection;

	waypointIsActivePreFrame_ = waypointIsActive_;

	auto view = registry.View<TransformRoutineComponent, TransformComponent, EditTag, EditSelectedTag>();
	for (auto e : view) {
		auto* routine = registry.GetComponent<TransformRoutineComponent>(e);
		auto* baseT = registry.GetComponent<TransformComponent>(e);
		if (routine->keyframes.empty()) {
			continue;
		}

		// waypointが削除された場合等に選択indexが範囲外にならないようにする
		selectedWaypointIndex_ = std::clamp(
			selectedWaypointIndex_, 0, static_cast<int>(routine->keyframes.size()) - 1);

		Math::Matrix4x4 parentWorld = GetParentWorld3D(registry, baseT);

		float imgX = sceneRect.x;
		float imgY = sceneRect.y;
		constexpr float margin = 8.0f;
		constexpr float buttonSizeY = 28.f;
		constexpr float buttonSizeX = 28.f;

		// waypoint番号ボタン列（既存のT/R/S/Routeツールバーの下に配置）
		ImVec2 waypointBarPos = ImVec2(imgX + margin, imgY + margin + buttonSizeY * 4 + margin);
		ImGui::SetCursorScreenPos(waypointBarPos);
		for (size_t i = 0; i < routine->keyframes.size(); ++i) {
			if (i > 0) {
				ImGui::SameLine();
			}
			ImGui::PushID(static_cast<int>(i));
			bool isSelected = (static_cast<int>(i) == selectedWaypointIndex_);
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
			}
			if (ImGui::Button(std::to_string(i).c_str(), ImVec2(buttonSizeX, buttonSizeY))) {
				selectedWaypointIndex_ = static_cast<int>(i);
			}
			if (isSelected) {
				ImGui::PopStyleColor();
			}
			ImGui::PopID();
		}

		// 選択中waypointの操作切り替え(T/R/S)
		static ImGuizmo::OPERATION currentWaypointOp = ImGuizmo::TRANSLATE;
		ImVec2 opBarPos = ImVec2(imgX + margin, imgY + margin + buttonSizeY * 4 + margin + buttonSizeY + margin);
		ImGui::SetCursorScreenPos(opBarPos);
		if (ImGui::Button("wT")) {
			currentWaypointOp = ImGuizmo::TRANSLATE;
			sTriggerButton = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("wR")) {
			currentWaypointOp = ImGuizmo::ROTATE;
			sTriggerButton = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("wS")) {
			currentWaypointOp = ImGuizmo::SCALE;
			sTriggerButton = true;
		}

		// 非選択waypointはマーカーとして描画し、クリックで選択を切り替える
		for (size_t i = 0; i < routine->keyframes.size(); ++i) {
			if (static_cast<int>(i) == selectedWaypointIndex_) {
				continue;
			}
			const auto& kf = routine->keyframes[i];
			Math::Matrix4x4 kfWorld = kf.MakeAffineMatrix4x4() * parentWorld;
			Math::Vector3 worldPos = kfWorld.GetTranslate();

			Math::Vector2 screenPos;
			if (!WorldToScreen(worldPos, viewProjection, sceneRect, screenPos)) {
				continue;
			}

			constexpr float kMarkerRadius = 6.0f;
			bool insideSceneView =
				screenPos.x >= sceneRect.x + kMarkerRadius && screenPos.x <= sceneRect.x + sceneRect.z - kMarkerRadius &&
				screenPos.y >= sceneRect.y + kMarkerRadius && screenPos.y <= sceneRect.y + sceneRect.w - kMarkerRadius;
			if (!insideSceneView) {
				continue;
			}

			ImVec2 center(screenPos.x, screenPos.y);
			ImGui::GetForegroundDrawList()->AddCircleFilled(center, 6.0f, IM_COL32(255, 200, 0, 255));

			ImGui::SetCursorScreenPos(ImVec2(center.x - 8.0f, center.y - 8.0f));
			ImGui::PushID(10000 + static_cast<int>(i));
			if (ImGui::InvisibleButton("waypointMarker", ImVec2(16.0f, 16.0f))) {
				selectedWaypointIndex_ = static_cast<int>(i);
				sTriggerButton = true;
			}
			ImGui::PopID();
		}

		// 選択中waypointだけギズモで編集する
		auto& kf = routine->keyframes[selectedWaypointIndex_];
		Math::Matrix4x4 world = kf.MakeAffineMatrix4x4() * parentWorld;

		ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentWaypointOp, ImGuizmo::WORLD, *(world.m));

		// world → 親のワールド行列基準のローカルへ変換（TransformRoutineSystemが書き込む空間と揃える）
		Math::Matrix4x4 parentInverse = parentWorld;
		parentInverse.Inverse();
		Math::Matrix4x4 local = world * parentInverse;

		float translate[3];
		float rotation[3];
		float scale[3];
		ImGuizmo::DecomposeMatrixToComponents(*(local.m), translate, rotation, scale);

		static Math::Vector3 oldTranslate;
		static Math::Quaternion oldRotation;
		static Math::Vector3 oldScale;

		switch (currentWaypointOp) {
		case ImGuizmo::TRANSLATE: {
			if (ImGuizmo::IsUsing()) {
				kf.translate = local.GetTranslate();
				waypointIsActive_ = true;
			} else {
				waypointIsActive_ = false;
			}

			if (waypointIsActive_ && !waypointIsActivePreFrame_) {
				oldTranslate = kf.translate;
			}
			if (!waypointIsActive_ && waypointIsActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(&kf.translate, oldTranslate, kf.translate)
				);
			}
			break;
		}
		case ImGuizmo::ROTATE: {
			if (ImGuizmo::IsUsing()) {
				constexpr float kDegreeToRadian = PI / 180.0f;
				Math::Matrix4x4 rotateMatrix;
				rotateMatrix.MakeRotate(Math::Vector3(
					rotation[0] * kDegreeToRadian, rotation[1] * kDegreeToRadian, rotation[2] * kDegreeToRadian));
				kf.rotation.FromRotationMatrix(rotateMatrix);
				waypointIsActive_ = true;
			} else {
				waypointIsActive_ = false;
			}

			if (waypointIsActive_ && !waypointIsActivePreFrame_) {
				oldRotation = kf.rotation;
			}
			if (!waypointIsActive_ && waypointIsActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Quaternion>>(&kf.rotation, oldRotation, kf.rotation)
				);
			}
			break;
		}
		case ImGuizmo::SCALE: {
			if (ImGuizmo::IsUsing()) {
				kf.scale.x = scale[0];
				kf.scale.y = scale[1];
				kf.scale.z = scale[2];
				waypointIsActive_ = true;
			} else {
				waypointIsActive_ = false;
			}

			if (waypointIsActive_ && !waypointIsActivePreFrame_) {
				oldScale = kf.scale;
			}
			if (!waypointIsActive_ && waypointIsActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(&kf.scale, oldScale, kf.scale)
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