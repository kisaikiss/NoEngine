#include "stdafx.h"
#include "FieldEditorSystem.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/FieldObject/Component/FieldPlacementComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/GameTag.h"
#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "externals/nlohmann/json.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include "engine/Functions/Renderer/Primitive.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
const char* kFieldTypeDefaultsPath = "resources/game/td_3105/RailData/FieldObjectTypeDefaults.json";

std::string MakeFieldPlacementPath(const std::string& stageName) {
	return "resources/game/td_3105/RailData/" + stageName + "_field_objects.json";
}

float ToRadian(float degree) {
	return degree * 3.14159265f / 180.0f;
}

No::Quaternion MakeRotationFromEulerDeg(const No::Vector3& eulerDeg) {
	No::Quaternion qx;
	qx.FromAxisAngle(No::Vector3::RIGHT, ToRadian(eulerDeg.x));
	No::Quaternion qy;
	qy.FromAxisAngle(No::Vector3::UP, ToRadian(eulerDeg.y));
	No::Quaternion qz;
	qz.FromAxisAngle(No::Vector3::FORWARD, ToRadian(eulerDeg.z));
	return qx * qy * qz;
}

std::unordered_map<std::string, FieldObjectTypeDefinition> BuildDefaultTypeDefinitions() {
	std::unordered_map<std::string, FieldObjectTypeDefinition> defs;

	FieldObjectTypeDefinition skydome{};
	skydome.type = FieldObjectType::Skydome;
	skydome.typeKey = "Skydome";
	skydome.modelKey = CommentBoutResourceKey::kSkydomeModel;
	skydome.hasCollision = false;
	skydome.occludesPlayerAttack = false;
	skydome.baseColliderAtUnitScale = { 1.0f, 1.0f, 1.0f };
	defs[skydome.typeKey] = skydome;

	FieldObjectTypeDefinition building{};
	building.type = FieldObjectType::Building;
	building.typeKey = "Building";
	building.modelKey = CommentBoutResourceKey::kBuildingModel;
	building.hasCollision = true;
	building.occludesPlayerAttack = true;
	building.baseColliderAtUnitScale = { 1.0f, 1.0f, 1.0f };
	defs[building.typeKey] = building;

	FieldObjectTypeDefinition ground{};
	ground.type = FieldObjectType::Ground;
	ground.typeKey = "Ground";
	ground.modelKey = CommentBoutResourceKey::kGroundModel;
	ground.hasCollision = true;
	ground.occludesPlayerAttack = false;
	ground.baseColliderAtUnitScale = { 1.0f, 1.0f, 1.0f };
	defs[ground.typeKey] = ground;

	return defs;
}

void EnsureRequiredDefinitions(std::unordered_map<std::string, FieldObjectTypeDefinition>& defs) {
	const auto required = BuildDefaultTypeDefinitions();
	for (const auto& pair : required) {
		if (defs.find(pair.first) == defs.end()) {
			defs[pair.first] = pair.second;
		}
	}
}

void LoadTypeDefinitions(std::unordered_map<std::string, FieldObjectTypeDefinition>& defs) {
	defs = BuildDefaultTypeDefinitions();

	std::ifstream ifs(kFieldTypeDefaultsPath);
	if (!ifs) {
		return;
	}

	nlohmann::json json;
	ifs >> json;
	if (!json.is_object() || !json.contains("types") || !json["types"].is_array()) {
		return;
	}

	for (const auto& node : json["types"]) {
		if (!node.is_object() || !node.contains("typeKey") || !node["typeKey"].is_string()) {
			continue;
		}

		FieldObjectTypeDefinition def{};
		def.typeKey = node["typeKey"].get<std::string>();
		if (def.typeKey == "Skydome") {
			def.type = FieldObjectType::Skydome;
		} else if (def.typeKey == "Building") {
			def.type = FieldObjectType::Building;
		} else if (def.typeKey == "Ground") {
			def.type = FieldObjectType::Ground;
		} else {
			def.type = FieldObjectType::Unknown;
		}

		if (node.contains("modelKey") && node["modelKey"].is_string()) {
			def.modelKey = node["modelKey"].get<std::string>();
		}
		if (node.contains("hasCollision") && node["hasCollision"].is_boolean()) {
			def.hasCollision = node["hasCollision"].get<bool>();
		}
		if (node.contains("occludesPlayerAttack") && node["occludesPlayerAttack"].is_boolean()) {
			def.occludesPlayerAttack = node["occludesPlayerAttack"].get<bool>();
		}
		if (node.contains("baseColliderAtUnitScale") && node["baseColliderAtUnitScale"].is_array() && node["baseColliderAtUnitScale"].size() >= 3) {
			def.baseColliderAtUnitScale.x = node["baseColliderAtUnitScale"][0].get<float>();
			def.baseColliderAtUnitScale.y = node["baseColliderAtUnitScale"][1].get<float>();
			def.baseColliderAtUnitScale.z = node["baseColliderAtUnitScale"][2].get<float>();
		}

		if (!def.typeKey.empty()) {
			defs[def.typeKey] = def;
		}
	}

	EnsureRequiredDefinitions(defs);
}

void SaveTypeDefinitions(const std::unordered_map<std::string, FieldObjectTypeDefinition>& defs) {
	nlohmann::json json;
	auto& arr = json["types"];
	arr = nlohmann::json::array();

	std::vector<std::string> keys;
	keys.reserve(defs.size());
	for (const auto& pair : defs) {
		keys.push_back(pair.first);
	}
	std::sort(keys.begin(), keys.end());

	for (const auto& key : keys) {
		const auto it = defs.find(key);
		if (it == defs.end()) {
			continue;
		}
		const FieldObjectTypeDefinition& def = it->second;
		nlohmann::json node;
		node["typeKey"] = def.typeKey;
		node["modelKey"] = def.modelKey;
		node["hasCollision"] = def.hasCollision;
		node["occludesPlayerAttack"] = def.occludesPlayerAttack;
		node["baseColliderAtUnitScale"] = {
			def.baseColliderAtUnitScale.x,
			def.baseColliderAtUnitScale.y,
			def.baseColliderAtUnitScale.z
		};
		arr.push_back(node);
	}

	std::ofstream ofs(kFieldTypeDefaultsPath);
	if (ofs) {
		ofs << json.dump(2);
	}
}

void ApplyFieldObjectVisual(
	No::Registry& registry,
	No::Entity entity,
	FieldPlacementComponent& placement,
	No::TransformComponent& transform,
	const GameResourceComponent& resources
) {
	transform.rotation = MakeRotationFromEulerDeg(placement.rotationEulerDeg);

	auto* mesh = registry.GetComponent<No::MeshComponent>(entity);
	if (!mesh) {
		mesh = registry.AddComponent<No::MeshComponent>(entity);
	}
	auto* material = registry.GetComponent<No::MaterialComponent>(entity);
	if (!material) {
		material = registry.AddComponent<No::MaterialComponent>(entity);
	}

	if (const auto* model = FindGameModel(resources, placement.modelKey)) {
		NoEngine::Asset::ModelLoader::GetModel(model->assetName, mesh);
		material->materials = NoEngine::Asset::ModelLoader::GetMaterial(model->assetName);
	}

	material->psoName = L"Renderer : Default PSO";
	material->psoId = NoEngine::Render::GetPSOID(material->psoName);
	material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
}

void ApplyFieldObjectCollision(No::Registry& registry, No::Entity entity, const FieldPlacementComponent& placement) {
	if (placement.hasCollision) {
		auto* collider = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		if (!collider) {
			collider = registry.AddComponent<CommentBoutCollision::Collider3DComponent>(entity);
		}
		collider->shapeType = CommentBoutCollision::ShapeType3D::Box;
		collider->useScaleAsBox = true;
		collider->boxSizeMultiplier = placement.colliderBaseAtUnitScale;
		collider->collisionLayer = CommentBout::CollisionLayer::CBGround;
		collider->collisionMask = CommentBout::CollisionLayer::CBEnemyBullet;

		auto* projected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		if (!projected) {
			projected = registry.AddComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		}
		projected->source3DEntity = entity;

		if (placement.occludesPlayerAttack) {
			if (!registry.Has<CBFieldOccluderTag>(entity)) {
				registry.AddComponent<CBFieldOccluderTag>(entity);
			}
		} else {
			if (registry.Has<CBFieldOccluderTag>(entity)) {
				registry.RemoveComponent<CBFieldOccluderTag>(entity);
			}
		}
	} else {
		if (registry.Has<CommentBoutCollision::ProjectedColliderComponent>(entity)) {
			registry.RemoveComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		}
		if (registry.Has<CommentBoutCollision::Collider3DComponent>(entity)) {
			registry.RemoveComponent<CommentBoutCollision::Collider3DComponent>(entity);
		}
		if (registry.Has<CBFieldOccluderTag>(entity)) {
			registry.RemoveComponent<CBFieldOccluderTag>(entity);
		}
	}
}

No::Entity CreateFieldObjectEntity(
	No::Registry& registry,
	const GameResourceComponent& resources,
	const FieldObjectTypeDefinition& def,
	const std::string& objectId,
	const std::string& displayName,
	const No::Vector3& position,
	const No::Vector3& scale,
	const No::Vector3& rotationEulerDeg
) {
	No::Entity entity = registry.GenerateEntity();
	registry.AddComponent<CBFieldObjectTag>(entity);

	auto* placement = registry.AddComponent<FieldPlacementComponent>(entity);
	placement->objectId = objectId;
	placement->displayName = displayName;
	placement->typeKey = def.typeKey;
	placement->modelKey = def.modelKey;
	placement->hasCollision = def.hasCollision;
	placement->occludesPlayerAttack = def.occludesPlayerAttack;
	placement->colliderBaseAtUnitScale = def.baseColliderAtUnitScale;
	placement->rotationEulerDeg = rotationEulerDeg;

	auto* transform = registry.AddComponent<No::TransformComponent>(entity);
	transform->translate = position;
	transform->scale = scale;

	ApplyFieldObjectVisual(registry, entity, *placement, *transform, resources);
	ApplyFieldObjectCollision(registry, entity, *placement);

	return entity;
}

void DestroyAllFieldObjects(No::Registry& registry) {
	std::vector<No::Entity> entities;
	for (auto entity : registry.View<CBFieldObjectTag, FieldPlacementComponent>()) {
		entities.push_back(entity);
	}
	for (auto entity : entities) {
		registry.DestroyEntity(entity);
	}
}

void SaveFieldPlacements(No::Registry& registry, const std::string& stageName) {
	nlohmann::json json;
	json["stageName"] = stageName;
	auto& arr = json["objects"];
	arr = nlohmann::json::array();

	for (auto entity : registry.View<CBFieldObjectTag, FieldPlacementComponent, No::TransformComponent>()) {
		auto* placement = registry.GetComponent<FieldPlacementComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!placement || !transform) {
			continue;
		}
		nlohmann::json node;
		node["objectId"] = placement->objectId;
		node["displayName"] = placement->displayName;
		node["typeKey"] = placement->typeKey;
		node["position"] = { transform->translate.x, transform->translate.y, transform->translate.z };
		node["scale"] = { transform->scale.x, transform->scale.y, transform->scale.z };
		node["rotationEulerDeg"] = { placement->rotationEulerDeg.x, placement->rotationEulerDeg.y, placement->rotationEulerDeg.z };
		arr.push_back(node);
	}

	std::ofstream ofs(MakeFieldPlacementPath(stageName));
	if (ofs) {
		ofs << json.dump(2);
	}
}

void LoadFieldPlacements(
	No::Registry& registry,
	const GameResourceComponent& resources,
	const std::unordered_map<std::string, FieldObjectTypeDefinition>& defs,
	const std::string& stageName,
	int& idCounter
) {
	DestroyAllFieldObjects(registry);

	std::ifstream ifs(MakeFieldPlacementPath(stageName));
	if (!ifs) {
		return;
	}

	nlohmann::json json;
	ifs >> json;
	if (!json.is_object() || !json.contains("objects") || !json["objects"].is_array()) {
		return;
	}

	for (const auto& node : json["objects"]) {
		if (!node.is_object()) {
			continue;
		}
		std::string typeKey = "";
		if (node.contains("typeKey") && node["typeKey"].is_string()) {
			typeKey = node["typeKey"].get<std::string>();
		}
		auto defIt = defs.find(typeKey);
		if (defIt == defs.end()) {
			continue;
		}

		No::Vector3 pos = No::Vector3::ZERO;
		No::Vector3 scale = No::Vector3::UNIT_SCALE;
		No::Vector3 rot = No::Vector3::ZERO;
		if (node.contains("position") && node["position"].is_array() && node["position"].size() >= 3) {
			pos = { node["position"][0].get<float>(), node["position"][1].get<float>(), node["position"][2].get<float>() };
		}
		if (node.contains("scale") && node["scale"].is_array() && node["scale"].size() >= 3) {
			scale = { node["scale"][0].get<float>(), node["scale"][1].get<float>(), node["scale"][2].get<float>() };
		}
		if (node.contains("rotationEulerDeg") && node["rotationEulerDeg"].is_array() && node["rotationEulerDeg"].size() >= 3) {
			rot = { node["rotationEulerDeg"][0].get<float>(), node["rotationEulerDeg"][1].get<float>(), node["rotationEulerDeg"][2].get<float>() };
		}

		std::string objectId = "field_obj_" + std::to_string(idCounter++);
		if (node.contains("objectId") && node["objectId"].is_string()) {
			objectId = node["objectId"].get<std::string>();
		}
		std::string displayName = objectId;
		if (node.contains("displayName") && node["displayName"].is_string()) {
			displayName = node["displayName"].get<std::string>();
		}

		CreateFieldObjectEntity(registry, resources, defIt->second, objectId, displayName, pos, scale, rot);
	}
}

void UpdateLoadedFieldObjects(
	No::Registry& registry,
	const GameResourceComponent& resources,
	const std::unordered_map<std::string, FieldObjectTypeDefinition>& defs
) {
	for (auto entity : registry.View<CBFieldObjectTag, FieldPlacementComponent, No::TransformComponent>()) {
		auto* placement = registry.GetComponent<FieldPlacementComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		if (!placement || !transform) {
			continue;
		}

		auto it = defs.find(placement->typeKey);
		if (it != defs.end()) {
			placement->modelKey = it->second.modelKey;
			placement->hasCollision = it->second.hasCollision;
			placement->occludesPlayerAttack = it->second.occludesPlayerAttack;
			placement->colliderBaseAtUnitScale = it->second.baseColliderAtUnitScale;
		}

		ApplyFieldObjectVisual(registry, entity, *placement, *transform, resources);
		ApplyFieldObjectCollision(registry, entity, *placement);
	}
}
}

void FieldEditorSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	GameResourceComponent* resources = nullptr;
	for (auto entity : registry.View<CBGameResourceTag, GameResourceComponent>()) {
		resources = registry.GetComponent<GameResourceComponent>(entity);
		if (resources) {
			break;
		}
	}
	if (!resources) {
		return;
	}

	std::string stageName = "Stage_01";
	for (auto entity : registry.View<RailCameraComponent>()) {
		auto* rail = registry.GetComponent<RailCameraComponent>(entity);
		if (rail && !rail->stageName.empty()) {
			stageName = rail->stageName;
			break;
		}
	}

	static const No::Registry* loadedRegistry = nullptr;
	static std::string loadedStageName;
	static std::unordered_map<std::string, FieldObjectTypeDefinition> typeDefinitions = BuildDefaultTypeDefinitions();
	static No::Entity selectedEntity = No::nullEntity;
	static int addTypeIndex = 0;
	static int idCounter = 0;

	if (loadedRegistry != &registry || loadedStageName != stageName) {
		LoadTypeDefinitions(typeDefinitions);
		LoadFieldPlacements(registry, *resources, typeDefinitions, stageName, idCounter);
		loadedRegistry = &registry;
		loadedStageName = stageName;
		selectedEntity = No::nullEntity;
	}

	UpdateLoadedFieldObjects(registry, *resources, typeDefinitions);
#ifdef USE_IMGUI
	ImGui::Begin("Field Editor");
	ImGui::Text("ステージ: %s", stageName.c_str());

	if (ImGui::Button("フィールド読込")) {
		LoadFieldPlacements(registry, *resources, typeDefinitions, stageName, idCounter);
		selectedEntity = No::nullEntity;
	}
	ImGui::SameLine();
	if (ImGui::Button("フィールド保存")) {
		SaveFieldPlacements(registry, stageName);
	}

	if (ImGui::Button("種別デフォルト読込")) {
		LoadTypeDefinitions(typeDefinitions);
	}
	ImGui::SameLine();
	if (ImGui::Button("種別デフォルト保存")) {
		SaveTypeDefinitions(typeDefinitions);
	}
	std::vector<std::string> typeKeys;
	for (const auto& pair : typeDefinitions) {
		typeKeys.push_back(pair.first);
	}
	std::sort(typeKeys.begin(), typeKeys.end());
	if (addTypeIndex >= static_cast<int>(typeKeys.size())) {
		addTypeIndex = 0;
	}

	if (!typeKeys.empty()) {
		std::vector<const char*> labels;
		for (const auto& key : typeKeys) {
			labels.push_back(key.c_str());
		}
		ImGui::Combo("追加種別", &addTypeIndex, labels.data(), static_cast<int>(labels.size()));
		if (ImGui::Button("フィールドオブジェクト追加")) {
			const std::string& key = typeKeys[addTypeIndex];
			auto it = typeDefinitions.find(key);
			if (it != typeDefinitions.end()) {
				const std::string objectId = "field_obj_" + std::to_string(idCounter++);
				const std::string displayName = key + "_" + std::to_string(idCounter);
				selectedEntity = CreateFieldObjectEntity(
					registry,
					*resources,
					it->second,
					objectId,
					displayName,
					No::Vector3::ZERO,
					No::Vector3::UNIT_SCALE,
					No::Vector3::ZERO
				);
			}
		}
	}

	ImGui::Separator();
	ImGui::Text("配置済みオブジェクト");
	std::vector<No::Entity> entities;
	for (auto entity : registry.View<CBFieldObjectTag, FieldPlacementComponent>()) {
		entities.push_back(entity);
	}
	std::sort(entities.begin(), entities.end(), [&registry](No::Entity a, No::Entity b) {
		auto* ca = registry.GetComponent<FieldPlacementComponent>(a);
		auto* cb = registry.GetComponent<FieldPlacementComponent>(b);
		if (!ca || !cb) {
			return a < b;
		}
		return ca->displayName < cb->displayName;
	});

	for (auto entity : entities) {
		auto* placement = registry.GetComponent<FieldPlacementComponent>(entity);
		if (!placement) {
			continue;
		}
		char label[256];
		std::snprintf(label, sizeof(label), "%s (%s)", placement->displayName.c_str(), placement->typeKey.c_str());
		if (ImGui::Selectable(label, selectedEntity == entity)) {
			selectedEntity = entity;
		}
	}

	if (selectedEntity != No::nullEntity && registry.Has<FieldPlacementComponent>(selectedEntity) && registry.Has<No::TransformComponent>(selectedEntity)) {
		auto* placement = registry.GetComponent<FieldPlacementComponent>(selectedEntity);
		auto* transform = registry.GetComponent<No::TransformComponent>(selectedEntity);
		if (placement && transform) {
			ImGui::Separator();
			ImGui::Text("選択中オブジェクト");

			char nameBuffer[128];
			std::snprintf(nameBuffer, sizeof(nameBuffer), "%s", placement->displayName.c_str());
			if (ImGui::InputText("表示名", nameBuffer, sizeof(nameBuffer))) {
				placement->displayName = nameBuffer;
			}

			ImGui::Text("種別: %s", placement->typeKey.c_str());
			ImGui::DragFloat3("位置", &transform->translate.x, 0.05f);
			ImGui::DragFloat3("スケール", &transform->scale.x, 0.05f, 0.01f, 500.0f);
			ImGui::DragFloat3("回転 (deg)", &placement->rotationEulerDeg.x, 0.1f, -360.0f, 360.0f);

			if (ImGui::Button("選択オブジェクト削除")) {
				registry.DestroyEntity(selectedEntity);
				selectedEntity = No::nullEntity;
			}
		}
	}

	ImGui::Separator();
	ImGui::Text("種別デフォルト設定");
	for (auto& pair : typeDefinitions) {
		FieldObjectTypeDefinition& def = pair.second;
		if (ImGui::TreeNode(def.typeKey.c_str())) {
			char modelKeyBuffer[128];
			std::snprintf(modelKeyBuffer, sizeof(modelKeyBuffer), "%s", def.modelKey.c_str());
			if (ImGui::InputText("モデルキー", modelKeyBuffer, sizeof(modelKeyBuffer))) {
				def.modelKey = modelKeyBuffer;
			}
			ImGui::Checkbox("当たり判定あり", &def.hasCollision);
			ImGui::Checkbox("プレイヤー攻撃を遮蔽", &def.occludesPlayerAttack);
			ImGui::DragFloat3("基準AABB @scale(1)", &def.baseColliderAtUnitScale.x, 0.01f, 0.01f, 100.0f);
			ImGui::TreePop();
		}
	}

	ImGui::Separator();
	ImGui::Text("遮蔽仕様(第1段階): プレイヤー攻撃のみ対象");
	ImGui::Text("敵弾遮蔽は将来拡張予定");
	ImGui::End();
#endif
}
