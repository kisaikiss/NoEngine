#include "EditUtils.h"
#include "engine/Editor/ComponentRegistry.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Runtime/Command/CommandContext.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Functions/Command/EditCommand/ChangeValueCommand.h"
#include "engine/Functions/Command/EditCommand/InstantiateEntityCommand.h"
#include "engine/Functions/Command/EditCommand/RemoveComponentCommand.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "DataDriven/PrefabSerializer.h"
#include "EditorCommandOperator.h"
#include "engine/Functions/Input/input.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {



namespace {
#ifdef USE_IMGUI
ImTextureID sGameTexture;
ImTextureID sSceneTexture;
Math::Vector4 sSceneTexRect;
std::function<void(const Math::Vector4& imageRect)> sGizmoCallback;
bool sIsMouseOverWindow = false;
Math::Vector2 sGameWindowMousePosition{};
bool sIsMouseOverSceneWindow = false;
Math::Vector2 sSceneWindowMousePosition{};
#endif // USE_IMGUI
}


Math::Vector2 Editor::Get2DGameWindowMousePosition(ECS::Registry& registry) {
	Math::Vector2 result{};
#ifdef USE_IMGUI
	result = sGameWindowMousePosition;
	result -= GraphicsCore::GetWindowSize() / 2.f;
	auto cameraView = registry.View<Component::ActiveCamera2DTag>();
	for (auto e : cameraView) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(e);

		result = Math::Vector2(result.x * transform->scale.x, result.y * transform->scale.y);
		result += transform->translate;
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

	return result;
}

bool Editor::IsMouseOverGameWindow() {
#ifdef USE_IMGUI
	return sIsMouseOverWindow;
#else
	return false;
#endif // USE_IMGUI

}

Math::Vector2 Editor::Get2DSceneWindowMousePosition(ECS::Registry& registry) {
	Math::Vector2 result{};
#ifdef USE_IMGUI
	result = sSceneWindowMousePosition;
	result -= GraphicsCore::GetWindowSize() / 2.f;
	auto cameraView = registry.View<Component::ActiveCamera2DTag>();
	for (auto e : cameraView) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(e);

		result = Math::Vector2(result.x * transform->scale.x, result.y * transform->scale.y);
		result += transform->translate;
	}
#else
	static_cast<void>(registry);
#endif // USE_IMGUI

	return result;
}

bool Editor::IsMouseOverSceneWindow() {

#ifdef USE_IMGUI
	return sIsMouseOverSceneWindow;
#else
	return false;
#endif // USE_IMGUI
}



void Editor::DrawGrid2D(Math::Vector2 gridSize) {
	static const float kGridNum = 1000;
	for (uint32_t i = 0; i < kGridNum; i++) {
		DebugPrimitive::DrawLine2D(Math::Vector2(gridSize.x * i - gridSize.x * kGridNum / 2.f, -gridSize.x * kGridNum), Math::Vector2(gridSize.x * i - gridSize.x * kGridNum / 2.f, gridSize.x * kGridNum), Math::Color::WHITE);
		DebugPrimitive::DrawLine2D(Math::Vector2(-gridSize.x * kGridNum, gridSize.y * i - gridSize.y * kGridNum / 2.f), Math::Vector2(gridSize.x * kGridNum, gridSize.y * i - gridSize.y * kGridNum / 2.f), Math::Color::WHITE);
	}

}

std::string MakeUniqueName(const std::unordered_set<std::string>& used, const std::string& base) {
	if (used.find(base) == used.end()) return base;
	// base が既に使われている場合は base_1, base_2 ... を試す
	int i = 1;
	while (true) {
		std::string cand = base + "_" + std::to_string(i);
		if (used.find(cand) == used.end()) return cand;
		++i;
	}
}

std::unordered_set<std::string> CollectEditTagNames(ECS::Registry& registry, ECS::Entity except) {
	std::unordered_set<std::string> names;
	auto view = registry.View<Editor::EditTag>();
	for (auto e : view) {
		if (e == except) continue;
		const Editor::EditTag* tag = registry.GetComponent<Editor::EditTag>(e);
		if (tag) names.insert(tag->name);
	}
	return names;
}

ECS::Entity PickObject(ECS::Registry& registry, ColorBuffer& idColorBuffer, ReadbackBuffer& readBackBuffer) {
#ifdef USE_IMGUI

	if (Input::Mouse::IsTrigger(Input::MouseButton::Left)) {
		if (!Editor::IsMouseOverSceneWindow()) {
			return ECS::INVALID_ENTITY;
		}
		(void)registry;
		CommandContext& ctx = CommandContext::Begin(L"PickObject Context");

		ctx.CopyPixelToBuffer(readBackBuffer, idColorBuffer, static_cast<UINT>(sSceneWindowMousePosition.x), static_cast<UINT>(sSceneWindowMousePosition.y));
		ctx.Finish(true);

		const uint8_t* pixelData = reinterpret_cast<const uint8_t*>(readBackBuffer.Map());

		// DXGI_FORMAT_R8G8B8A8_UNORM はメモリ上では R, G, B, A の順で1バイトずつ格納されます
		uint8_t r = pixelData[0];
		uint8_t g = pixelData[1];
		uint8_t b = pixelData[2];
		uint8_t a = pixelData[3];

		readBackBuffer.Unmap();

		// RGB値からオブジェクトIDを復元する
		uint32_t pickedID = (r << 24) | (g << 16) | (b << 8) | a;
		constexpr uint32_t kNoObjectNumber = 1724754687;
		if (pickedID == kNoObjectNumber) {
			return ECS::INVALID_ENTITY;
		}

		return static_cast<ECS::Entity>(pickedID);
	}
#else
	static_cast<void>(registry);
	static_cast<void>(idColorBuffer);
	static_cast<void>(readBackBuffer);

#endif // USE_IMGUI

	return ECS::INVALID_ENTITY;
}

void InitGameImGuiWindow(ColorBuffer& mainColor) {
#ifdef USE_IMGUI
	mainColor.CreateImGuiSRV();
	{
		NoEngine::DescriptorHandle slot = Render::gTextureHeap.Alloc();
		GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(
			1,
			static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(slot),
			mainColor.GetImGuiSRV(),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		sGameTexture = static_cast<ImTextureID>(slot.GetGpuPtr());
	}
#else
	static_cast<void>(mainColor);
#endif // USE_IMGUI
}

void InitSceneImGuiWindow(ColorBuffer& debugColor) {
#ifdef USE_IMGUI
	debugColor.CreateImGuiSRV();
	{
		NoEngine::DescriptorHandle slot = Render::gTextureHeap.Alloc();
		GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(
			1,
			static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(slot),
			debugColor.GetImGuiSRV(),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		sSceneTexture = static_cast<ImTextureID>(slot.GetGpuPtr());
	}
#else
	static_cast<void>(debugColor);
#endif // USE_IMGUI
}

void DrawGameImGuiWindow() {
#ifdef USE_IMGUI
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	ImGui::Begin("Game");
	ImGui::Image(
		sGameTexture,
		ImVec2(windowSize.x * 2 / 3, windowSize.y * 2 / 3) // 表示サイズ
	);

	sIsMouseOverWindow = false;
	if (ImGui::IsItemHovered()) {
		ImVec2 imgPos = ImGui::GetItemRectMin();
		ImVec2 imgSize = ImGui::GetItemRectSize();
		ImVec2 mouse = ImGui::GetMousePos();

		ImVec2 local(mouse.x - imgPos.x, mouse.y - imgPos.y);

		if (0 <= local.x && local.x < imgSize.x &&
			0 <= local.y && local.y < imgSize.y) {
			ImVec2 uv(local.x / imgSize.x, local.y / imgSize.y);

			// ウィンドウ内のスクリーン座標に変換
			sGameWindowMousePosition.x = uv.x * windowSize.x;
			sGameWindowMousePosition.y = uv.y * windowSize.y;
			sIsMouseOverWindow = true;
		}
	}

	ImGui::End();
#endif // USE_IMGUI
}

void DrawSceneImGuiWindow(ECS::Registry& registry) {
#ifdef USE_IMGUI
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	ImGui::Begin("Scene");
	ImGui::Image(
		sSceneTexture,
		ImVec2(windowSize.x * 2 / 3, windowSize.y * 2 / 3) // 表示サイズ
	);

	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	sSceneTexRect = Math::Vector4(min.x, min.y, max.x - min.x, max.y - min.y);
	
	sIsMouseOverSceneWindow = false;
	if (ImGui::IsItemHovered()) {
		ImVec2 imgPos = ImGui::GetItemRectMin();
		ImVec2 imgSize = ImGui::GetItemRectSize();
		ImVec2 mouse = ImGui::GetMousePos();

		ImVec2 local(mouse.x - imgPos.x, mouse.y - imgPos.y);

		if (0 <= local.x && local.x < imgSize.x &&
			0 <= local.y && local.y < imgSize.y) {
			ImVec2 uv(local.x / imgSize.x, local.y / imgSize.y);

			// ウィンドウ内のスクリーン座標に変換
			sSceneWindowMousePosition.x = uv.x * windowSize.x;
			sSceneWindowMousePosition.y = uv.y * windowSize.y;
			sIsMouseOverSceneWindow = true;
		}
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB_PATH")) {
			const char* path = (const char*)payload->Data;
			// インスタンス化
			auto e = Editor::InstantiatePreset(registry, std::string(path));
			registry.AddComponent<Editor::EditSelectedTag>(e);

			// Transform があるなら
			if (auto* t = registry.GetComponent<Component::TransformComponent>(e)) {
				// ToDo: ドロップ位置からレイを飛ばし、衝突判定を取った場所に配置する

				// カメラの向いている方向から50m離れた場所に配置
				auto cameraView = registry.View<Component::CameraComponent, Component::DebugCameraComponent>();
				Component::CameraComponent* camera = nullptr;
				Component::TransformComponent* transform = nullptr;
				for (auto entity : cameraView) {
					camera = registry.GetComponent<Component::CameraComponent>(entity);
					transform = registry.GetComponent<Component::TransformComponent>(entity);
				}
				if (camera && transform) {
					Math::Vector3 worldPos = camera->forGPU.worldPosition;
					const float kOffset = 50.f;
					worldPos += transform->rotation.zAxis() * kOffset;
					t->translate = worldPos;
				}

			}
			// Undo 用コマンド登録
			Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::InstantiateEntityCommand>(registry, e));
		}
		ImGui::EndDragDropTarget();
	}

	if (sGizmoCallback) sGizmoCallback(sSceneTexRect);
	ImGui::End();
#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void SetGizmoCallback(std::function<void(const Math::Vector4& imageRect)> cb) {
#ifdef USE_IMGUI
	sGizmoCallback = cb;
#else
	static_cast<void>(cb);
#endif // USE_IMGUI

}

using namespace ECS;
void DrawComponentUI(Registry& registry, Entity e) {
#ifdef USE_IMGUI
	for (auto& compInfo : ComponentRegistry::GetAll()) {
		if (!registry.Has(compInfo.typeId, e))
			continue;

		if (ImGui::CollapsingHeader(compInfo.name.c_str())) {
			void* compPtr = compInfo.getter(registry, e);

			for (auto& field : compInfo.fields) {
				DrawFieldUI(field, compPtr);
			}
			// 削除ボタン
			if (ImGui::SmallButton("Remove")) {
				ImGui::OpenPopup("ConfirmRemove");
			}
			if (ImGui::BeginPopupModal("ConfirmRemove")) {
				ImGui::Text("Remove this component?");
				if (ImGui::Button("Yes")) {
					Editor::EditorCommandOperator::AddCommand(std::make_unique<Command::RemoveComponentCommand>(registry, e, compInfo.typeId));
					registry.RemoveComponent(compInfo.typeId, e);
				}
				ImGui::SameLine();
				if (ImGui::Button("No")) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}
#else
	static_cast<void>(registry);
	static_cast<void>(e);
#endif // USE_IMGUI
}

void DrawFieldUI(const FieldInfo& field, void* ptr) {
#ifdef USE_IMGUI
	if (!field.attributes.editable) {
		return;
	}
	uint8_t* base = (uint8_t*)ptr;
	void* valuePtr = base + field.offset;
	switch (field.type) {
	case FieldType::Float: {
		float* fPtr = static_cast<float*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static float oldFloatValue;

		if (field.attributes.hasRange) {
			ImGui::DragFloat(field.name.c_str(), fPtr, field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat(field.name.c_str(), fPtr, field.attributes.valueSpeed);
		}

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldFloatValue = *fPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<float>>(fPtr, oldFloatValue, *fPtr)
			);
		}
		break;
	}
	case FieldType::Float2: {
		Math::Vector2* vPtr = static_cast<Math::Vector2*>(valuePtr);

		static Math::Vector2 oldVector2Value;

		if (field.attributes.hasRange) {
			ImGui::DragFloat2(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat2(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}


		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldVector2Value = *vPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<Math::Vector2>>(vPtr, oldVector2Value, *vPtr)
			);
		}
		break;
	}
	case FieldType::Float3: {
		Math::Vector3* vPtr = static_cast<Math::Vector3*>(valuePtr);

		static Math::Vector3 oldVectorValue;

		if (field.attributes.hasRange) {
			ImGui::DragFloat3(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat3(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}


		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldVectorValue = *vPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<Math::Vector3>>(vPtr, oldVectorValue, *vPtr)
			);
		}
		break;
	}
	case FieldType::Float4: {
		Math::Vector4* vPtr = static_cast<Math::Vector4*>(valuePtr);

		static Math::Vector4 oldVectorValue;

		if (field.attributes.hasRange) {
			ImGui::DragFloat4(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat4(field.name.c_str(), static_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}


		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldVectorValue = *vPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<Math::Vector4>>(vPtr, oldVectorValue, *vPtr)
			);
		}
		break;
	}
	case FieldType::Int: {
		int* iPtr = static_cast<int*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static int oldIntValue;

		if (field.attributes.hasRange) {
			ImGui::DragInt(field.name.c_str(), static_cast<int*>(valuePtr), field.attributes.valueSpeed, static_cast<int>(field.attributes.minValue), static_cast<int>(field.attributes.maxValue));
		} else {
			ImGui::DragInt(field.name.c_str(), static_cast<int*>(valuePtr), field.attributes.valueSpeed);
		}

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldIntValue = *iPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<int>>(iPtr, oldIntValue, *iPtr)
			);
		}
		break;
	}
	case FieldType::Uint: {
		uint32_t* iPtr = static_cast<uint32_t*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static uint32_t oldIntValue;

		ImGui::DragInt(field.name.c_str(), static_cast<int*>(valuePtr), field.attributes.valueSpeed, 0, INT32_MAX);

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldIntValue = *iPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<uint32_t>>(iPtr, oldIntValue, *iPtr)
			);
		}
		break;
	}
	case FieldType::Bool: {
		bool *bPtr = static_cast<bool*>(valuePtr);

		// 一時保存用の変数（同時に編集できるUIは1つなのでstaticで使い回せます）
		static bool oldBoolValue;

		ImGui::Checkbox(field.name.c_str(), static_cast<bool*>(valuePtr));

		// 編集が開始された瞬間（マウスでクリックした時など）に元の値を保存
		if (ImGui::IsItemActivated()) {
			oldBoolValue = *bPtr;
		}

		// 編集が確定した瞬間（マウスを離した時、Enterを押した時など）にコマンドを発行
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			Editor::EditorCommandOperator::AddCommand(
				std::make_unique<Command::ChangeValueCommand<bool>>(bPtr, oldBoolValue, *bPtr)
			);
		}
		break;
	}
	case FieldType::String: {
		std::string* s = static_cast<std::string*>(valuePtr);

		// 編集中の状態を保持するためのstatic変数
		static char editBuf[1024] = "";
		static std::string oldStringValue;
		static void* activePtr = nullptr; // 現在編集中の変数のポインタ

		// このフィールドが現在アクティブ（編集中）でなければ、実際の値(*s)をバッファに同期する
		// （ポインタで判定することで、他のStringフィールド描画時に上書きされるのを防ぎます）
		if (activePtr != s) {
			strncpy_s(editBuf, s->c_str(), sizeof(editBuf));
			editBuf[sizeof(editBuf) - 1] = '\0';
		}

		// Enterを押したタイミングを検知するためのフラグ
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

		// InputTextを描画（入力中は editBuf のみが書き換わり、*s は変更されません）
		bool enterPressed = ImGui::InputText(field.name.c_str(), editBuf, sizeof(editBuf), flags);

		// 編集が開始された瞬間（クリックした時など）
		if (ImGui::IsItemActivated()) {
			oldStringValue = *s;
			activePtr = s; // この変数を編集中としてマーク
		}

		// Enterが押された、またはフォーカスが外れて編集が完了した時
		if (enterPressed || ImGui::IsItemDeactivatedAfterEdit()) {
			if (*s != editBuf) {
				// 変更があればコマンドを積んでから、実際の値に反映
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<std::string>>(s, oldStringValue, editBuf)
				);
				*s = editBuf;
			}
		}

		// 編集状態が解除された時（Enter確定後、フォーカス外れ、Escキーでのキャンセル等）
		if (ImGui::IsItemDeactivated()) {
			activePtr = nullptr; // マークを外す
		}

		break;
	}
	case FieldType::WString: {
		std::wstring* s = static_cast<std::wstring*>(valuePtr);

		// 編集中の状態を保持するためのstatic変数
		static char editBuf[1024] = "";
		static std::wstring oldStringValue;
		static void* activePtr = nullptr; // 現在編集中の変数のポインタ

		// このフィールドが現在アクティブ（編集中）でなければ、実際の値(*s)をバッファに同期する
		// （ポインタで判定することで、他のStringフィールド描画時に上書きされるのを防ぎます）
		if (activePtr != s) {
			strncpy_s(editBuf, ConvertString(s->c_str()).c_str(), sizeof(editBuf));
			editBuf[sizeof(editBuf) - 1] = '\0';
		}

		// Enterを押したタイミングを検知するためのフラグ
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

		// InputTextを描画（入力中は editBuf のみが書き換わり、*s は変更されません）
		bool enterPressed = ImGui::InputText(field.name.c_str(), editBuf, sizeof(editBuf), flags);

		// 編集が開始された瞬間（クリックした時など）
		if (ImGui::IsItemActivated()) {
			oldStringValue = *s;
			activePtr = s; // この変数を編集中としてマーク
		}

		// Enterが押された、またはフォーカスが外れて編集が完了した時
		if (enterPressed || ImGui::IsItemDeactivatedAfterEdit()) {
			if (ConvertString(*s) != editBuf) {
				// 変更があればコマンドを積んでから、実際の値に反映
				Editor::EditorCommandOperator::AddCommand(
					std::make_unique<Command::ChangeValueCommand<std::wstring>>(s, oldStringValue, ConvertString(editBuf))
				);
				*s = ConvertString(editBuf);
			}
		}

		// 編集状態が解除された時（Enter確定後、フォーカス外れ、Escキーでのキャンセル等）
		if (ImGui::IsItemDeactivated()) {
			activePtr = nullptr; // マークを外す
		}

		break;
	}
	default:
		ImGui::Text(field.name.c_str());
		ImGui::SameLine();
		ImGui::Text(" : Unsupported field type");
		break;
	}

#else
	static_cast<void>(field);
	static_cast<void>(ptr);
#endif // USE_IMGUI

}
}

