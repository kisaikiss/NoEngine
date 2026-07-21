#include "stdafx.h"
#include "SceneSerializer.h"
#include "engine/Functions/Scene/SceneNameComponent.h"
#include "../EditUtils.h"

namespace NoEngine {
namespace Editor {
using namespace nlohmann;
json SaveScene(ECS::Registry& registry) {
	json scene;

	auto editView = registry.View<EditTag>();
	for (auto entity : editView) {
		auto* tag = registry.GetComponent<EditTag>(entity);
		const std::string& name = tag->name;

		scene["entities"][name] = SaveEntityToJson(registry, entity);
	}


	return scene;
}

json SaveEntityToJson(ECS::Registry& registry, ECS::Entity entity) {
	json result;
	for (auto& typeInfo : ComponentRegistry::GetAll()) {
		if (!registry.Has(typeInfo.typeId, entity)) continue;

		void* compPtr = typeInfo.getter(registry, entity);

		json compJson;
		for (auto& field : typeInfo.fields) {
			uint8_t* base = (uint8_t*)compPtr + field.offset;
			WriteFieldToJson(compJson, field, base);
		}

		result["components"][typeInfo.name] = compJson;

	}


	return result;
}

void WriteFieldToJson(nlohmann::json& j, const FieldInfo& field, void* ptr) {
	switch (field.type) {
	case NoEngine::FieldType::Float:
		j[field.name] = *(float*)ptr;
		break;
	case NoEngine::FieldType::Float2:
		j[field.name] = { ((float*)ptr)[0], ((float*)ptr)[1] };
		break;
	case NoEngine::FieldType::Float3:
		j[field.name] = { ((float*)ptr)[0], ((float*)ptr)[1], ((float*)ptr)[2] };
		break;
	case NoEngine::FieldType::Float4:
		j[field.name] = { ((float*)ptr)[0], ((float*)ptr)[1], ((float*)ptr)[2], ((float*)ptr)[3] };
		break;
	case NoEngine::FieldType::Int:
		j[field.name] = *(int*)ptr;
		break;
	case NoEngine::FieldType::Uint:
		j[field.name] = *(int*)ptr;
		break;
	case NoEngine::FieldType::Bool:
		j[field.name] = *(bool*)ptr;
		break;
	case NoEngine::FieldType::String: {
		// std::string を指している想定
		const std::string* s = reinterpret_cast<const std::string*>(ptr);
		j[field.name] = *s;
	}
		break;
	case NoEngine::FieldType::WString: {
		// std::wstring を指している想定
		const std::wstring* s = reinterpret_cast<const std::wstring*>(ptr);
		j[field.name] = *s;
	}
		break;
	case NoEngine::FieldType::Enum:
		j[field.name] = field.enumOps->toString(ptr);
		break;
	case NoEngine::FieldType::Struct: {
		TypeInfo* nested = field.structTypeInfo ? field.structTypeInfo() : nullptr;
		if (nested) {
			json structJson;
			for (auto& subField : nested->fields) {
				uint8_t* subPtr = (uint8_t*)ptr + subField.offset;
				WriteFieldToJson(structJson, subField, subPtr);
			}
			j[field.name] = structJson;
		}
		break;
	}
	case NoEngine::FieldType::Array: {
		json arrJson = json::array();
		size_t count = field.arrayOps->size ? field.arrayOps->size(ptr) : 0;
		for (size_t i = 0; i < count; ++i) {
			void* elemPtr = field.arrayOps->getElement(ptr, i);
			arrJson.push_back(WriteArrayElementToJson(field, elemPtr));
		}
		j[field.name] = arrJson;
		break;
	}
	default:
		j[field.name] = "Unsupported";
		break;
	}
}

void LoadScene(ECS::Registry& registry, const json& scene) {
	const auto& entities = scene["entities"];
	for (auto& [name, entityJson] : entities.items()) {

		// 名前でEntityを探す
		ECS::Entity e = FindEntityByName(registry, name);

		if (e == ECS::INVALID_ENTITY) {
			e = registry.GenerateEntity();
			registry.AddComponent<EditTag>(e)->name = name;
		}

		// Componentを復元
		LoadEntityFromJson(registry, e, entityJson);
	}

}

ECS::Entity FindEntityByName(ECS::Registry& registry, const std::string& name) {
	auto editView = registry.View<EditTag>();
	for (auto entity : editView) {
		auto* tag = registry.GetComponent<EditTag>(entity);
		if (tag->name == name) {
			return entity;
		}
	}
	return ECS::INVALID_ENTITY;

}

void LoadEntityFromJson(ECS::Registry& registry, ECS::Entity entity, const json& j) {
	for (auto& [compName, compJson] : j["components"].items()) {
		TypeInfo* typeInfo = ComponentRegistry::FindByName(compName);
		if (!typeInfo) continue;

		void* compPtr = registry.GetComponent(typeInfo->typeId, entity);
		if (!compPtr) {
			compPtr = registry.AddComponent(typeInfo->typeId, entity);
		}
		for (auto& field : typeInfo->fields) {
			uint8_t* base = (uint8_t*)compPtr + field.offset;
			ReadFieldFromJson(compJson, field, base);
		}
	}
}

void ReadFieldFromJson(const nlohmann::json& j, const FieldInfo& field, void* ptr) {
	if (!j.contains(field.name)) return;
	switch (field.type) {
	case NoEngine::FieldType::Float:
		*(float*)ptr = j[field.name].get<float>();
		break;
	case NoEngine::FieldType::Float2:
	{
		auto arr = j[field.name];
		float* v = (float*)ptr;
		v[0] = arr[0];
		v[1] = arr[1];
	}
		break;
	case NoEngine::FieldType::Float3:
	{
		auto arr = j[field.name];
		float* v = (float*)ptr;
		v[0] = arr[0];
		v[1] = arr[1];
		v[2] = arr[2];
	}
	break;
	case NoEngine::FieldType::Float4:
	{
		auto arr = j[field.name];
		float* v = (float*)ptr;
		v[0] = arr[0];
		v[1] = arr[1];
		v[2] = arr[2];
		v[3] = arr[3];
	}
		break;
	case NoEngine::FieldType::Int:
		*(int*)ptr = j[field.name].get<int>();
		break;
	case NoEngine::FieldType::Uint:
		*(uint32_t*)ptr = j[field.name].get<uint32_t>();
		break;
	case NoEngine::FieldType::Bool:
		*(bool*)ptr = j[field.name].get<bool>();
		break;
	case NoEngine::FieldType::String: {
		const std::string value = j[field.name].get<std::string>();
		// std::string に書き込む
		std::string* s = reinterpret_cast<std::string*>(ptr);
		*s = value;
		break;
	}
	case NoEngine::FieldType::WString: {
		const std::wstring value = j[field.name].get<std::wstring>();
		// std::string に書き込む
		std::wstring* s = reinterpret_cast<std::wstring*>(ptr);
		*s = value;
		break;
	}
	case NoEngine::FieldType::Enum: {
		const std::string value = j[field.name].get<std::string>();
		field.enumOps->fromString(ptr, value);
		break;
	}
	case NoEngine::FieldType::Struct: {
		TypeInfo* nested = field.structTypeInfo ? field.structTypeInfo() : nullptr;
		if (nested) {
			const json& structJson = j[field.name];
			for (auto& subField : nested->fields) {
				uint8_t* subPtr = (uint8_t*)ptr + subField.offset;
				ReadFieldFromJson(structJson, subField, subPtr);
			}
		}
		break;
	}
	case NoEngine::FieldType::Array: {
		if (!field.arrayOps) break;
		const json& arrJson = j[field.name];

		// 既存要素をクリアしてからJSON側の要素で作り直す
		while (field.arrayOps->size(ptr) > 0) {
			field.arrayOps->removeElement(ptr, field.arrayOps->size(ptr) - 1);
		}
		for (size_t i = 0; i < arrJson.size(); ++i) {
			field.arrayOps->insertElement(ptr, i);
			void* elemPtr = field.arrayOps->getElement(ptr, i);
			ReadArrayElementFromJson(arrJson[i], field, elemPtr);
		}
		break;
	}
	default:
		break;
	}
}

nlohmann::json WriteArrayElementToJson(const FieldInfo& arrayField, void* elemPtr) {
	if (arrayField.arrayOps->elementType == FieldType::Struct) {
		TypeInfo* nested = arrayField.arrayOps->elementStructTypeInfo ? arrayField.arrayOps->elementStructTypeInfo() : nullptr;
		json elemJson;
		if (nested) {
			for (auto& subField : nested->fields) {
				uint8_t* subPtr = (uint8_t*)elemPtr + subField.offset;
				WriteFieldToJson(elemJson, subField, subPtr);
			}
		}
		return elemJson;
	}
	// プリミティブ要素は既存のWriteFieldToJsonを"value"キーで呼び、中身だけ取り出す
	FieldInfo tmp{};
	tmp.name = "value";
	tmp.type = arrayField.arrayOps->elementType;
	json wrapper;
	WriteFieldToJson(wrapper, tmp, elemPtr);
	return wrapper["value"];
}

void ReadArrayElementFromJson(const nlohmann::json& elemJson, const FieldInfo& arrayField, void* elemPtr) {
	if (arrayField.arrayOps->elementType == FieldType::Struct) {
		TypeInfo* nested = arrayField.arrayOps->elementStructTypeInfo ? arrayField.arrayOps->elementStructTypeInfo() : nullptr;
		if (!nested) return;
		for (auto& subField : nested->fields) {
			uint8_t* subPtr = (uint8_t*)elemPtr + subField.offset;
			ReadFieldFromJson(elemJson, subField, subPtr);
		}
		return;
	}
	FieldInfo tmp{};
	tmp.name = "value";
	tmp.type = arrayField.arrayOps->elementType;
	json wrapper;
	wrapper["value"] = elemJson;
	ReadFieldFromJson(wrapper, tmp, elemPtr);
}

}
}