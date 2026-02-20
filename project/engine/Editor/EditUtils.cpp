#include "EditUtils.h"
#include "engine/Editor/ComponentRegistry.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
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
