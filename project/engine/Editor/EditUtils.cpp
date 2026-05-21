#include "EditUtils.h"
#include "engine/Editor/ComponentRegistry.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "ReflectionMacros.h"
#include "engine/Functions/Renderer/RenderSystem.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {

REFLECT_STRUCT_BEGIN(NoEngine::Editor::EditTag)
REFLECT_FIELD(name),
REFLECT_FIELD(path)
REFLECT_STRUCT_END(NoEngine::Editor::EditTag)


namespace {
#ifdef USE_IMGUI
ImTextureID sGameTexture;
ImTextureID sSceneTexture;
bool sIsMouseOverWindow = false;
Math::Vector2 sGameWindowMousePosition{};
#endif // USE_IMGUI
}


Math::Vector2 Editor::Get2DSceneMousePosition(ECS::Registry& registry) {
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

bool Editor::IsMouseOverSceneWindow() {
#ifdef USE_IMGUI
	return sIsMouseOverWindow;
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

void DrawSceneImGuiWindow() {
#ifdef USE_IMGUI
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	ImGui::Begin("Scene");
	ImGui::Image(
		sSceneTexture,
		ImVec2(windowSize.x * 2 / 3, windowSize.y * 2 / 3) // 表示サイズ
	);
	ImGui::End();
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
	case FieldType::Float:
		if (field.attributes.hasRange) {
			ImGui::DragFloat(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}
		break;
	case FieldType::Float2:
		if (field.attributes.hasRange) {
			ImGui::DragFloat2(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat2(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}
		break;
	case FieldType::Float3:
		if (field.attributes.hasRange) {
			ImGui::DragFloat3(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat3(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}
		break;
	case FieldType::Float4:
		if (field.attributes.hasRange) {
			ImGui::DragFloat4(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed, field.attributes.minValue, field.attributes.maxValue);
		} else {
			ImGui::DragFloat4(field.name.c_str(), reinterpret_cast<float*>(valuePtr), field.attributes.valueSpeed);
		}
		break;
	case FieldType::Int:
		if (field.attributes.hasRange) {
			ImGui::DragInt(field.name.c_str(), reinterpret_cast<int*>(valuePtr), field.attributes.valueSpeed, static_cast<int>(field.attributes.minValue), static_cast<int>(field.attributes.maxValue));
		} else {
			ImGui::DragInt(field.name.c_str(), reinterpret_cast<int*>(valuePtr), field.attributes.valueSpeed);
		}
		break;
	case FieldType::Bool:
		ImGui::Checkbox(field.name.c_str(), reinterpret_cast<bool*>(valuePtr));
		break;
	case FieldType::String: {
		// valuePtr が std::string* を指している場合
		std::string* s = reinterpret_cast<std::string*>(valuePtr);

		// 一時バッファを用意して ImGui に渡す
		const size_t BUF_SIZE = 1024; // 必要に応じて調整
		char buf[BUF_SIZE];
		strncpy_s(buf, s->c_str(), BUF_SIZE);
		buf[BUF_SIZE - 1] = '\0';

		if (ImGui::InputText(field.name.c_str(), buf, BUF_SIZE)) {
			// 変更があれば std::string に戻す
			*s = buf;
		}
	}

						  break;
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

