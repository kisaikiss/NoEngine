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
	case NoEngine::FieldType::Bool:
		j[field.name] = *(bool*)ptr;
		break;
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

		if (e == ECS::nullEntity) {
			return;
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
	return ECS::nullEntity;

}

void LoadEntityFromJson(ECS::Registry& registry, ECS::Entity entity, const json& j) {
	for (auto& [compName, compJson] : j["components"].items()) {
		TypeInfo* typeInfo = ComponentRegistry::FindByName(compName);
		if (!typeInfo) continue;

		void* compPtr = registry.GetComponent(typeInfo->typeId, entity);

		for (auto& field : typeInfo->fields) {
			uint8_t* base = (uint8_t*)compPtr + field.offset;
			base;
			ReadFieldFromJson(compJson, field, base);
		}
	}
}

void ReadFieldFromJson(const nlohmann::json& j, const FieldInfo& field, void* ptr) {
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
		break;
	case NoEngine::FieldType::Int:
		*(int*)ptr = j[field.name].get<int>();
		break;
	case NoEngine::FieldType::Bool:
		*(bool*)ptr = j[field.name].get<bool>();
		break;
	default:
		break;
	}
}



}
}