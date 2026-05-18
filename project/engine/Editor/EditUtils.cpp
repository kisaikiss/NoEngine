#include "EditUtils.h"
#include "engine/Editor/ComponentRegistry.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Renderer/Primitive.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {


Math::Vector2 Editor::Get2DSceneMousePosition(ECS::Registry& registry) {
	Math::Vector2 result{};
#ifdef USE_IMGUI
	result = GraphicsCore::GetSceneWindowMousePosition();
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
	return GraphicsCore::IsMouseOverSceneWindow();
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

