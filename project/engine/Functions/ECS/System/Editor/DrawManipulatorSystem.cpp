#include "stdafx.h"
#include "DrawManipulatorSystem.h"
#include "engine/Editor/Commands/EditTag.h"
#include "engine/Editor/Utilities/EditUtils.h"
#include "engine/Editor/Utilities/ScenePlacement.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformRoutineComponent2D.h"
#include "engine/Functions/Command/EditCommand/ChangeValueCommand.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Runtime/GraphicsCore.h"

#ifdef USE_IMGUI
#include "externals/imgui/ImGuizmo.h"
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
namespace ECS {

namespace {
bool sTriggerButton = false;
int  sSelectedWaypointIndex = 0;    // 現在ギズモで掴んでいるwaypointの添字
int  sSelectedWaypointIndex2D = 0; 
Entity sVector3GizmoEntity = INVALID_ENTITY;
Math::Vector3* sVector3GizmoValue = nullptr;

#ifdef USE_IMGUI
struct GizmoSettings {
	ImGuizmo::MODE mode = ImGuizmo::WORLD;
	ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	bool snapEnabled = false;
	float translationSnap[3] = { 1.f, 1.f, 1.f };
	float rotationSnap = 15.f; // ImGuizmoには度数法で渡す。
	float scaleSnap = 0.1f;

	const float* GetSnap(ImGuizmo::OPERATION requestedOperation) const {
		if (!snapEnabled) return nullptr;
		switch (requestedOperation) {
		case ImGuizmo::TRANSLATE: return translationSnap;
		case ImGuizmo::ROTATE: return &rotationSnap;
		case ImGuizmo::SCALE: return &scaleSnap;
		default: return nullptr;
		}
	}
};
GizmoSettings sGizmoSettings;

// 設定はボタンを押したときだけ開き、通常はシーンを広く表示する。
bool DrawGizmoSettings(const Math::Vector4& rect, bool* routeMode, int* waypointIndex, size_t waypointCount) {
	const float panelWidth = std::max(1.f, std::min(330.f, rect.z - 16.f));
	const float panelHeight = std::max(1.f, rect.w - 16.f);
	ImGui::SetCursorScreenPos(ImVec2(rect.x + 8.f, rect.y + 8.f));
	// 閉じるためのクリックを、背後のオブジェクト選択へ通さない。
	bool blocked = ImGui::IsPopupOpen("GizmoSettingsPopup");
	ImGui::BeginDisabled(ImGuizmo::IsUsingAny());
	if (ImGui::Button("Gizmo")) ImGui::OpenPopup("GizmoSettingsPopup");
	blocked |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)
		|| ImGui::IsItemActive() || ImGui::IsItemDeactivated();
	const ImVec2 buttonMin = ImGui::GetItemRectMin();
	const ImVec2 buttonMax = ImGui::GetItemRectMax();
	ImGui::EndDisabled();
	ImGui::SetNextWindowPos(ImVec2(buttonMin.x, buttonMax.y + 4.f), ImGuiCond_Appearing);
	ImGui::SetNextWindowSizeConstraints(ImVec2(panelWidth, 0.f), ImVec2(panelWidth, panelHeight));
	if (ImGui::BeginPopup("GizmoSettingsPopup", ImGuiWindowFlags_NoMove)) {
		blocked = true;
		ImGui::BeginDisabled(ImGuizmo::IsUsingAny());
		auto trackInteraction = [&blocked] {
			blocked |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)
				|| ImGui::IsItemActive() || ImGui::IsItemDeactivated();
		};
		ImGui::TextUnformatted("Gizmo");
		const char* operationLabels[] = { "Move", "Rotate", "Scale" };
		const ImGuizmo::OPERATION operations[] = { ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE };
		for (int i = 0; i < 3; ++i) {
			if (i > 0) ImGui::SameLine();
			if (ImGui::RadioButton(operationLabels[i], sGizmoSettings.operation == operations[i])) {
				sGizmoSettings.operation = operations[i];
			}
			trackInteraction();
		}
		if (ImGui::RadioButton("World", sGizmoSettings.mode == ImGuizmo::WORLD)) {
			sGizmoSettings.mode = ImGuizmo::WORLD;
		}
		trackInteraction();
		ImGui::SameLine();
		if (ImGui::RadioButton("Local", sGizmoSettings.mode == ImGuizmo::LOCAL)) {
			sGizmoSettings.mode = ImGuizmo::LOCAL;
		}
		trackInteraction();
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move/rotate in object axes. Scale always uses local axes.");
		ImGui::SameLine();
		ImGui::Checkbox("Snap", &sGizmoSettings.snapEnabled);
		trackInteraction();
		const bool showSnapSettings = ImGui::TreeNode("Snap settings");
		trackInteraction();
		if (showSnapSettings) {
			auto drawStep = [&trackInteraction](const char* label, float& value) {
				ImGui::SetNextItemWidth(85.f);
				ImGui::DragFloat(label, &value, 0.01f, 0.001f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
				trackInteraction();
				// テキスト入力による非有限値やゼロ以下の刻み幅も除外する。
				if (!std::isfinite(value) || value <= 0.f) value = 0.001f;
			};
			drawStep("Move step", sGizmoSettings.translationSnap[0]);
			drawStep("Rotate step (deg)", sGizmoSettings.rotationSnap);
			drawStep("Scale step", sGizmoSettings.scaleSnap);
			ImGui::TreePop();
		}
		sGizmoSettings.translationSnap[1] = sGizmoSettings.translationSnap[0];
		sGizmoSettings.translationSnap[2] = sGizmoSettings.translationSnap[0];

		if (routeMode && waypointIndex) {
			ImGui::Separator();
			ImGui::TextUnformatted("Routine / Edit target");
			if (ImGui::RadioButton("Object", !*routeMode)) *routeMode = false;
			trackInteraction();
			ImGui::SameLine();
			if (ImGui::RadioButton("Route", *routeMode)) *routeMode = true;
			trackInteraction();
			if (waypointCount == 0) {
				ImGui::TextDisabled("No keyframes. Add them in Inspector.");
			} else {
				*waypointIndex = std::clamp(*waypointIndex, 0, static_cast<int>(waypointCount) - 1);
				ImGui::BeginDisabled(!*routeMode);
				ImGui::Text("Selected index: %d   Total: %d", *waypointIndex, static_cast<int>(waypointCount));
				ImGui::SetNextItemWidth(-1.f);
				const std::string preview = "Keyframe [" + std::to_string(*waypointIndex) + "]";
				const bool comboOpen = ImGui::BeginCombo("##Keyframe", preview.c_str(), ImGuiComboFlags_HeightRegular);
				trackInteraction();
				if (comboOpen) {
					for (int i = 0; i < static_cast<int>(waypointCount); ++i) {
						const std::string label = "Keyframe [" + std::to_string(i) + "]";
						if (ImGui::Selectable(label.c_str(), i == *waypointIndex)) *waypointIndex = i;
						trackInteraction();
						if (i == *waypointIndex) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::BeginDisabled(*waypointIndex == 0);
				if (ImGui::Button("< Previous")) --*waypointIndex;
				trackInteraction();
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::BeginDisabled(*waypointIndex == static_cast<int>(waypointCount) - 1);
				if (ImGui::Button("Next >")) ++*waypointIndex;
				trackInteraction();
				ImGui::EndDisabled();
				ImGui::EndDisabled();
			}
		}
		ImGui::Separator();
		if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
		trackInteraction();
		ImGui::EndDisabled();
		ImGui::EndPopup();
	}
	// ボタン登録後のカーソルを維持し、アイテム未登録の座標復元を避ける。
	return blocked;
}
#endif
}

using namespace Editor;
using namespace Component;

void DrawManipulatorSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

#ifdef USE_IMGUI
	sTriggerButton = false;
	SetGizmoCallback([this, &registry](const Math::Vector4& rect) {
		// 配置ドラッグ中は、既存オブジェクトのギズモ操作と設定表示を止める。
		if (Editor::IsScenePlacementActive()) {
			sTriggerButton = true;
			return;
		}
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(rect.x, rect.y, rect.z, rect.w);
		bool settingsBlocked = false;
		for (auto e : registry.View<EditTag, EditSelectedTag>()) {
			if (registry.Has<TransformComponent>(e) || registry.Has<Transform2DComponent>(e)) {
				bool* routeMode = nullptr;
				int* waypointIndex = nullptr;
				size_t waypointCount = 0;
				if (registry.Has<TransformComponent>(e)) {
					if (auto* routine = registry.GetComponent<TransformRoutineComponent>(e)) {
						routeMode = &routineEditMode_;
						waypointIndex = &sSelectedWaypointIndex;
						waypointCount = routine->keyframes.size();
					}
				} else if (auto* routine = registry.GetComponent<TransformRoutineComponent2D>(e)) {
					routeMode = &routineEditMode2D_;
					waypointIndex = &sSelectedWaypointIndex2D;
					waypointCount = routine->keyframes.size();
				}
				settingsBlocked = DrawGizmoSettings(rect, routeMode, waypointIndex, waypointCount);
				break;
			}
		}
		sTriggerButton |= settingsBlocked;
		// パネルのクリックやドラッグで背後のギズモをつかまないようにする。
		ImGuizmo::Enable(!settingsBlocked || ImGuizmo::IsUsingAny());
		Manipulate3D(registry, rect);
		Manipulate2D(registry, rect);
		ManipulateRoutineWaypoints(registry, rect);
		ManipulateRoutineWaypoints2D(registry, rect);
		});

#else
	static_cast<void>(registry);
#endif // USE_IMGUI

}

bool DrawManipulatorSystem::TriggerManipulateButton() {
	return sTriggerButton;
}

void DrawManipulatorSystem::SetSelectWaypointIndex(int index) {
	sSelectedWaypointIndex = index;
}

void DrawManipulatorSystem::SetSelectWaypointIndex2D(int index) {
	sSelectedWaypointIndex2D = index;
}

void DrawManipulatorSystem::ToggleVector3Gizmo(Entity entity, Math::Vector3* value) {
	if (sVector3GizmoEntity == entity && sVector3GizmoValue == value) {
		sVector3GizmoEntity = INVALID_ENTITY;
		sVector3GizmoValue = nullptr;
		return;
	}
	sVector3GizmoEntity = entity;
	sVector3GizmoValue = value;
}

bool DrawManipulatorSystem::IsVector3GizmoTarget(Entity entity, const Math::Vector3* value) {
	return sVector3GizmoEntity == entity && sVector3GizmoValue == value;
}

void DrawManipulatorSystem::Manipulate3D(Registry& registry, const Math::Vector4& sceneRect) {
	static_cast<void>(sceneRect);

#ifdef USE_IMGUI
	ImGuizmo::SetOrthographic(false);
	Math::Matrix4x4 viewMatrix;
	Math::Matrix4x4 projection;
	GetActiveCamera3D(registry, viewMatrix, projection);

	isActivePreFrame_ = isActive_;
	auto view = registry.View<TransformComponent, EditTag, EditSelectedTag>();
	for (auto e : view) {
		const auto currentOp = sGizmoSettings.operation;
		bool hasRoutine = registry.Has<TransformRoutineComponent>(e);

		// waypoint編集モード中は、このエンティティ自体のTransformは動かさない
		// (1フレームに2つのImGuizmo::Manipulateを重ねて呼ぶと競合するため)
		if (hasRoutine && routineEditMode_) {
			continue;
		}

		auto* t = registry.GetComponent<TransformComponent>(e);
		Math::Matrix4x4 m = t->MakeAffineMatrix4x4(registry);

		// Inspectorで選んだVector3だけをギズモで操作する。未選択時は通常のTransformギズモを使う。
		if (sVector3GizmoEntity == e && sVector3GizmoValue) {
			static Math::Vector3 oldGizmoValue;
			static bool vectorGizmoWasUsing = false;
			Math::Matrix4x4 fieldMatrix;
			fieldMatrix.MakeTranslate(*sVector3GizmoValue);
			ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), ImGuizmo::TRANSLATE,
				ImGuizmo::WORLD, *(fieldMatrix.m), nullptr, sGizmoSettings.GetSnap(ImGuizmo::TRANSLATE));
			const bool isUsingVectorGizmo = ImGuizmo::IsUsing();
			if (isUsingVectorGizmo) {
				if (!vectorGizmoWasUsing) oldGizmoValue = *sVector3GizmoValue;
				*sVector3GizmoValue = fieldMatrix.GetTranslate();
			}
			if (!isUsingVectorGizmo && vectorGizmoWasUsing) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(
						sVector3GizmoValue, oldGizmoValue, *sVector3GizmoValue));
				sVector3GizmoEntity = INVALID_ENTITY;
				sVector3GizmoValue = nullptr;
			}
			vectorGizmoWasUsing = isUsingVectorGizmo;
			isActive_ = false;
			continue;
		}

		ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentOp,
			sGizmoSettings.mode, *(m.m), nullptr, sGizmoSettings.GetSnap(currentOp));

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
			if (ImGuizmo::IsUsing() && !isActivePreFrame_) {
				oldVector = t->translate;
			}
			if (ImGuizmo::IsUsing()) {
				t->translate = localMatrix.GetTranslate();
				isActive_ = true;
			} else {
				isActive_ = false;
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
			if (ImGuizmo::IsUsing() && !isActivePreFrame_) {
				oldQuaternion = t->rotation;
			}
			if (ImGuizmo::IsUsing()) {
				constexpr float kDegreeToRadian = PI / 180.0f;
				Math::Matrix4x4 rotateMatrix;
				rotateMatrix.MakeRotate(Math::Vector3(rotation[0] * kDegreeToRadian, rotation[1] * kDegreeToRadian, rotation[2] * kDegreeToRadian));
				t->rotation.FromRotationMatrix(rotateMatrix);
				isActive_ = true;
			} else {
				isActive_ = false;
			}

			if (!isActive_ && isActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Quaternion>>(&t->rotation, oldQuaternion, t->rotation)
				);
			}
			break;
		}
		case ImGuizmo::SCALE: {
			if (ImGuizmo::IsUsing() && !isActivePreFrame_) {
				oldVector = t->scale;
			}
			if (ImGuizmo::IsUsing()) {
				t->scale.x = scale[0];
				t->scale.y = scale[1];
				t->scale.z = scale[2];
				isActive_ = true;
			} else {
				isActive_ = false;
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
	static_cast<void>(sceneRect);

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
		const auto currentOp = sGizmoSettings.operation;
		bool hasRoutine2D = registry.Has<TransformRoutineComponent2D>(e);

		// waypoint編集モード中は、このエンティティ自体のTransformは動かさない
		if (hasRoutine2D && routineEditMode2D_) {
			continue;
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

		ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentOp,
			sGizmoSettings.mode, *(m.m), nullptr, sGizmoSettings.GetSnap(currentOp));

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
			if (ImGuizmo::IsUsing() && !isActivePreFrame_) {
				oldVector = t->translate;
			}
			if (ImGuizmo::IsUsing()) {
				t->translate.x = localMatrix.GetTranslate().x;
				t->translate.y = localMatrix.GetTranslate().y;
				isActive_ = true;
			} else {
				isActive_ = false;
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
			if (ImGuizmo::IsUsing() && !isActivePreFrame_) {
				oldRotate = t->rotation;
			}
			if (ImGuizmo::IsUsing()) {
				constexpr float kDegreeToRadian = PI / 180.0f;
				t->rotation = rotation[2] * kDegreeToRadian;
				isActive_ = true;
			} else {
				isActive_ = false;
			}

			if (!isActive_ && isActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<float>>(&t->rotation, oldRotate, t->rotation)
				);
			}
			break;
		}
		case ImGuizmo::SCALE: {
			if (ImGuizmo::IsUsing() && !isActivePreFrame_) {
				oldVector = t->scale;
			}
			if (ImGuizmo::IsUsing()) {
				t->scale.x = scale[0];
				t->scale.y = scale[1];
				isActive_ = true;
			} else {
				isActive_ = false;
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

Math::Matrix4x4 DrawManipulatorSystem::GetParentWorld2D(Registry& registry, Component::Transform2DComponent* t) {
	if (t->parent != ECS::INVALID_ENTITY) {
		if (auto* parentTransform = registry.GetComponent<Transform2DComponent>(t->parent)) {
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
		sSelectedWaypointIndex = std::clamp(
			sSelectedWaypointIndex, 0, static_cast<int>(routine->keyframes.size()) - 1);

		Math::Matrix4x4 parentWorld = GetParentWorld3D(registry, baseT);

		const auto currentWaypointOp = sGizmoSettings.operation;

		// 非選択waypointはマーカーとして描画し、クリックで選択を切り替える
		for (size_t i = 0; i < routine->keyframes.size(); ++i) {
			if (static_cast<int>(i) == sSelectedWaypointIndex) {
				continue;
			}
			const auto& kf = routine->keyframes[i];
			Math::Matrix4x4 kfWorld = kf.MakeAffineMatrix4x4(registry) * parentWorld;
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
			ImGui::GetWindowDrawList()->AddCircleFilled(center, 6.0f, IM_COL32(255, 200, 0, 255));

			// 操作できないマーカーでは、アイテム登録を伴わないカーソル移動も行わない。
			if (sTriggerButton || ImGuizmo::IsUsingAny()) continue;
			ImGui::SetCursorScreenPos(ImVec2(center.x - 8.0f, center.y - 8.0f));
			ImGui::PushID(10000 + static_cast<int>(i));
			if (ImGui::InvisibleButton("waypointMarker", ImVec2(16.0f, 16.0f))) {
				sSelectedWaypointIndex = static_cast<int>(i);
				sTriggerButton = true;
			}
			ImGui::PopID();
		}

		// 選択中waypointだけギズモで編集する
		auto& kf = routine->keyframes[sSelectedWaypointIndex];
		Math::Matrix4x4 world = kf.MakeAffineMatrix4x4(registry) * parentWorld;

		ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentWaypointOp,
			sGizmoSettings.mode, *(world.m), nullptr, sGizmoSettings.GetSnap(currentWaypointOp));

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
			if (ImGuizmo::IsUsing() && !waypointIsActivePreFrame_) {
				oldTranslate = kf.translate;
			}
			if (ImGuizmo::IsUsing()) {
				kf.translate = local.GetTranslate();
				waypointIsActive_ = true;
			} else {
				waypointIsActive_ = false;
			}

			if (!waypointIsActive_ && waypointIsActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(&kf.translate, oldTranslate, kf.translate)
				);
			}
			break;
		}
		case ImGuizmo::ROTATE: {
			if (ImGuizmo::IsUsing() && !waypointIsActivePreFrame_) {
				oldRotation = kf.rotation;
			}
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

			if (!waypointIsActive_ && waypointIsActivePreFrame_) {
				EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Quaternion>>(&kf.rotation, oldRotation, kf.rotation)
				);
			}
			break;
		}
		case ImGuizmo::SCALE: {
			if (ImGuizmo::IsUsing() && !waypointIsActivePreFrame_) {
				oldScale = kf.scale;
			}
			if (ImGuizmo::IsUsing()) {
				kf.scale.x = scale[0];
				kf.scale.y = scale[1];
				kf.scale.z = scale[2];
				waypointIsActive_ = true;
			} else {
				waypointIsActive_ = false;
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

void DrawManipulatorSystem::ManipulateRoutineWaypoints2D(Registry& registry, const Math::Vector4& sceneRect) {
#ifdef USE_IMGUI
	if (!routineEditMode2D_) {
		return;
	}

	ImGuizmo::SetOrthographic(true);

	// --- Manipulate2Dと同じカメラ/空間セットアップ ---
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

	auto windowSize = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	Math::Matrix4x4 screenViewMatrix = Math::Matrix4x4::IDENTITY;
	Math::Matrix4x4 screenProjection = MathCalculations::MakeOrthographicMatrix(
		0.f, 0.f, static_cast<float>(windowSize.clientWidth), static_cast<float>(windowSize.clientHeight), 0.1f, 100.f);

	waypointIsActivePreFrame2D_ = waypointIsActive2D_;

	auto view = registry.View<TransformRoutineComponent2D, Transform2DComponent, EditTag, EditSelectedTag>();
	for (auto e : view) {
		auto* routine = registry.GetComponent<TransformRoutineComponent2D>(e);
		auto* baseT = registry.GetComponent<Transform2DComponent>(e);
		if (routine->keyframes.empty()) {
			continue;
		}

		sSelectedWaypointIndex2D = std::clamp(
			sSelectedWaypointIndex2D, 0, static_cast<int>(routine->keyframes.size()) - 1);

		Math::Matrix4x4 parentWorld = GetParentWorld2D(registry, baseT);

		// このエンティティ自身がScreen空間UIかどうかで、waypoint操作の基準空間を切り替える
		auto* sprite = registry.GetComponent<SpriteComponent>(e);
		bool isScreenSpace = sprite && sprite->space == SpriteSpace::Screen;

		Math::Vector2 anchorOffset = { 0.f, 0.f };
		if (isScreenSpace) {
			anchorOffset.x = sprite->anchor.x * static_cast<float>(windowSize.clientWidth);
			anchorOffset.y = sprite->anchor.y * static_cast<float>(windowSize.clientHeight);
		}

		const Math::Matrix4x4& viewMatrix = isScreenSpace ? screenViewMatrix : worldViewMatrix;
		const Math::Matrix4x4& projection = isScreenSpace ? screenProjection : worldProjection;
		Math::Matrix4x4 viewProjection = viewMatrix * projection;

		const auto currentWaypointOp2D = sGizmoSettings.operation;

		// 非選択waypointはマーカーとして描画し、クリックで選択を切り替える
		for (size_t i = 0; i < routine->keyframes.size(); ++i) {
			if (static_cast<int>(i) == sSelectedWaypointIndex2D) continue;

			const auto& kf = routine->keyframes[i];
			Math::Matrix4x4 kfWorld = kf.MakeAffineMatrix4x4(registry) * parentWorld;
			Math::Vector3 worldPos = kfWorld.GetTranslate();
			if (isScreenSpace) {
				worldPos.x += anchorOffset.x;
				worldPos.y += anchorOffset.y;
			} else if (existCamera) {
				worldPos.x += static_cast<float>(size.clientWidth) / 2.f * cameraSize.x;
				worldPos.y += static_cast<float>(size.clientHeight) / 2.f * cameraSize.y;
			}

			Math::Vector2 screenPos;
			if (!WorldToScreen(worldPos, viewProjection, sceneRect, screenPos)) continue;

			constexpr float kMarkerRadius = 6.0f;
			bool insideSceneView =
				screenPos.x >= sceneRect.x + kMarkerRadius && screenPos.x <= sceneRect.x + sceneRect.z - kMarkerRadius &&
				screenPos.y >= sceneRect.y + kMarkerRadius && screenPos.y <= sceneRect.y + sceneRect.w - kMarkerRadius;
			if (!insideSceneView) continue;

			ImVec2 center(screenPos.x, screenPos.y);
			ImGui::GetWindowDrawList()->AddCircleFilled(center, 6.0f, IM_COL32(255, 200, 0, 255));

			// 操作できないマーカーでは、アイテム登録を伴わないカーソル移動も行わない。
			if (sTriggerButton || ImGuizmo::IsUsingAny()) continue;
			ImGui::SetCursorScreenPos(ImVec2(center.x - 8.0f, center.y - 8.0f));
			ImGui::PushID(20000 + static_cast<int>(i));
			if (ImGui::InvisibleButton("waypointMarker2D", ImVec2(16.0f, 16.0f))) {
				sSelectedWaypointIndex2D = static_cast<int>(i);
				sTriggerButton = true;
			}
			ImGui::PopID();
		}

		// 選択中waypointだけギズモで編集する
		auto& kf = routine->keyframes[sSelectedWaypointIndex2D];
		Math::Matrix4x4 world = kf.MakeAffineMatrix4x4(registry) * parentWorld;

		if (isScreenSpace) {
			world.m[3][0] += anchorOffset.x;
			world.m[3][1] += anchorOffset.y;
		} else if (existCamera) {
			world.m[3][0] += static_cast<float>(size.clientWidth) / 2.f * cameraSize.x;
			world.m[3][1] += static_cast<float>(size.clientHeight) / 2.f * cameraSize.y;
		}

		ImGuizmo::Manipulate(*(viewMatrix.m), *(projection.m), currentWaypointOp2D,
			sGizmoSettings.mode, *(world.m), nullptr, sGizmoSettings.GetSnap(currentWaypointOp2D));

		if (isScreenSpace) {
			world.m[3][0] -= anchorOffset.x;
			world.m[3][1] -= anchorOffset.y;
		} else if (existCamera) {
			world.m[3][0] -= static_cast<float>(size.clientWidth) / 2.f * cameraSize.x;
			world.m[3][1] -= static_cast<float>(size.clientHeight) / 2.f * cameraSize.y;
		}

		// world → 親のワールド行列基準のローカルへ変換（TransformRoutineSystem2Dが書き込む空間と揃える）
		Math::Matrix4x4 parentInverse = parentWorld;
		parentInverse.Inverse();
		Math::Matrix4x4 local = world * parentInverse;

		float translate[3];
		float rotation[3];
		float scale[3];
		ImGuizmo::DecomposeMatrixToComponents(*(local.m), translate, rotation, scale);

		static Math::Vector2 oldTranslate2D;
		static float oldRotation2D;
		static Math::Vector2 oldScale2D;

		switch (currentWaypointOp2D) {
		case ImGuizmo::TRANSLATE: {
			if (ImGuizmo::IsUsing() && !waypointIsActivePreFrame2D_) {
				oldTranslate2D = kf.translate;
			}
			if (ImGuizmo::IsUsing()) {
				kf.translate.x = local.GetTranslate().x;
				kf.translate.y = local.GetTranslate().y;
				waypointIsActive2D_ = true;
			} else {
				waypointIsActive2D_ = false;
			}
			if (!waypointIsActive2D_ && waypointIsActivePreFrame2D_) {
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector2>>(&kf.translate, oldTranslate2D, kf.translate)
				);
			}
			break;
		}
		case ImGuizmo::ROTATE: {
			if (ImGuizmo::IsUsing() && !waypointIsActivePreFrame2D_) {
				oldRotation2D = kf.rotation;
			}
			if (ImGuizmo::IsUsing()) {
				constexpr float kDegreeToRadian = PI / 180.0f;
				kf.rotation = rotation[2] * kDegreeToRadian; // Transform2Dの回転はZ軸まわりのfloat(ラジアン)
				waypointIsActive2D_ = true;
			} else {
				waypointIsActive2D_ = false;
			}
			if (!waypointIsActive2D_ && waypointIsActivePreFrame2D_) {
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<float>>(&kf.rotation, oldRotation2D, kf.rotation)
				);
			}
			break;
		}
		case ImGuizmo::SCALE: {
			if (ImGuizmo::IsUsing() && !waypointIsActivePreFrame2D_) {
				oldScale2D = kf.scale;
			}
			if (ImGuizmo::IsUsing()) {
				kf.scale.x = scale[0];
				kf.scale.y = scale[1];
				waypointIsActive2D_ = true;
			} else {
				waypointIsActive2D_ = false;
			}
			if (!waypointIsActive2D_ && waypointIsActivePreFrame2D_) {
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<Math::Vector2>>(&kf.scale, oldScale2D, kf.scale)
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

#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::DrawManipulatorSystem, "DrawManipulatorSystem", "Editor")
