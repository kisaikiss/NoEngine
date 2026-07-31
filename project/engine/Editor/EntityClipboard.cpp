#include "EntityClipboard.h"
#include "EntityHierarchyUtils.h"
#include "engine/Editor/ReflectionMacros.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"

namespace NoEngine {
namespace Editor {

void EntityClipboard::Copy(ECS::Registry& registry, ECS::Entity root) {
	if (root == ECS::INVALID_ENTITY) return;

	std::vector<ECS::Entity> subtree;
	CollectSubtreeEntities(registry, root, subtree);

	// Entity -> subtree内でのインデックス（貼り付け時に親子関係を復元するため）
	std::unordered_map<ECS::Entity, int> indexOf;
	indexOf.reserve(subtree.size() * 2);
	for (size_t i = 0; i < subtree.size(); ++i) {
		indexOf[subtree[i]] = static_cast<int>(i);
	}

	nlohmann::json nodesJson = nlohmann::json::array();
	for (auto e : subtree) {
		auto* tag = registry.GetComponent<Editor::EditTag>(e);
		nlohmann::json nodeJson;
		nodeJson["entity"] = Editor::SaveEntityToJson(registry, e);
		auto it = (tag && tag->parent != ECS::INVALID_ENTITY) ? indexOf.find(tag->parent) : indexOf.end();
		nodeJson["parentIndex"] = (it != indexOf.end()) ? it->second : -1;
		nodesJson.push_back(nodeJson);
	}

	copyObject_ = nlohmann::json{ {"nodes", nodesJson} };

	auto* rootTag = registry.GetComponent<Editor::EditTag>(root);
	LogInfo("CopyObject name : " + (rootTag ? rootTag->name : std::string())
		+ " (" + std::to_string(subtree.size()) + " entities)");
}

std::vector<ECS::Entity> EntityClipboard::Paste(ECS::Registry& registry) {
	std::vector<ECS::Entity> newEntities;
	if (!HasContent()) return newEntities;

	const auto& nodes = copyObject_["nodes"];
	newEntities.reserve(nodes.size());

	// 1) まず全Entityを生成してデータを流し込む
	for (const auto& node : nodes) {
		ECS::Entity newE = registry.GenerateEntity();
		Editor::LoadEntityFromJson(registry, newE, node["entity"]);
		newEntities.push_back(newE);
	}

	// 2) 新しいEntity IDで親子関係を張り直す
	for (size_t i = 0; i < newEntities.size(); ++i) {
		int parentIndex = nodes[i].value("parentIndex", -1);
		ECS::Entity newParent = (parentIndex >= 0) ? newEntities[static_cast<size_t>(parentIndex)] : ECS::INVALID_ENTITY;
		SetEntityParent(registry, newEntities[i], newParent);
	}

	return newEntities;
}

}
}
