#include "stdafx.h"
#include "EventLinkSystem.h"
#include "engine/Functions/ECS/Event/GameplayEvents.h"
#include "engine/Functions/ECS/Component/Common/EventLinkComponent.h"
#include "engine/Functions/ECS/Component/Common/GimmickGraphComponent.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"
#include "engine/Functions/ECS/Gameplay/GimmickGraph.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"

namespace NoEngine::ECS {
namespace {
using namespace Gameplay;

struct CachedGraph {
	std::filesystem::file_time_type lastWriteTime{};
	bool hasTimestamp = false;
	std::optional<GimmickGraph> graph;
};

std::unordered_map<std::string, CachedGraph>& GetGraphCache() {
	static std::unordered_map<std::string, CachedGraph> cache;
	return cache;
}

const GimmickGraph* GetGraph(const std::string& graphPath) {
	std::error_code error;
	const auto normalizedPath = std::filesystem::path(graphPath).lexically_normal();
	const std::string key = normalizedPath.generic_string();
	const auto timestamp = std::filesystem::last_write_time(normalizedPath, error);
	auto& cache = GetGraphCache()[key];
	if (error) {
		if (!cache.hasTimestamp) {
			LogWarning("EventLinkSystem: could not read graph file timestamp: " + key);
		}
		cache.graph.reset();
		cache.hasTimestamp = false;
		return nullptr;
	}
	if (cache.hasTimestamp && cache.lastWriteTime == timestamp) return cache.graph ? &*cache.graph : nullptr;

	cache.lastWriteTime = timestamp;
	cache.hasTimestamp = true;
	std::string loadError;
	cache.graph = LoadGimmickGraph(normalizedPath, &loadError);
	if (!cache.graph) {
		LogWarning("EventLinkSystem: could not load graph '" + key + "': " + loadError);
		return nullptr;
	}
	const auto validation = ValidateGimmickGraph(*cache.graph);
	if (validation.HasErrors()) {
		for (const auto& diagnostic : validation.diagnostics) {
			if (diagnostic.severity == DiagnosticSeverity::Error) {
				LogWarning("EventLinkSystem: invalid graph '" + key + "': " + diagnostic.message);
			}
		}
		cache.graph.reset();
		return nullptr;
	}
	return &*cache.graph;
}

void ExecuteGraph(Registry& registry, const Event::GameplayEvent& event, const GimmickGraph& graph) {
	const auto& catalog = GameplayCatalog::Get();
	std::unordered_map<std::string, const GimmickNode*> nodesById;
	for (const auto& node : graph.nodes) nodesById.emplace(node.id, &node);

	constexpr size_t kMaxActionsPerEvent = 1024;
	size_t executedActions = 0;
	for (const auto& eventNode : graph.nodes) {
		if (eventNode.kind != GimmickNodeKind::Event || eventNode.typeId != event.eventId) continue;
		std::vector<std::string> pending;
		for (auto it = graph.links.rbegin(); it != graph.links.rend(); ++it) {
			if (it->fromNodeId == eventNode.id && it->fromPinId == PinId::ExecOutput) pending.push_back(it->toNodeId);
		}
		while (!pending.empty()) {
			if (executedActions >= kMaxActionsPerEvent) {
				LogWarning("EventLinkSystem: graph execution limit exceeded for " + event.eventId);
				return;
			}
			const std::string nodeId = std::move(pending.back());
			pending.pop_back();
			auto nodeIt = nodesById.find(nodeId);
			if (nodeIt == nodesById.end()) continue;
			const GimmickNode& node = *nodeIt->second;
			if (node.kind != GimmickNodeKind::Action) continue;
			const ActionDescriptor* action = catalog.FindAction(node.typeId);
			if (!action) continue;

			Entity target = INVALID_ENTITY;
			for (const auto& property : action->properties) {
				if (property.type != ActionPropertyType::EntityReference) continue;
				auto valueIt = node.properties.find(property.id);
				if (valueIt == node.properties.end() || !valueIt->is_string()) continue;
				target = Editor::FindEntityByName(registry, valueIt->get<std::string>());
				if (target == INVALID_ENTITY && property.required) {
					LogWarning("EventLinkSystem: graph action target entity was not found: " + valueIt->get<std::string>());
				}
				break;
			}
			if (action->requiresTarget && target == INVALID_ENTITY) continue;
			++executedActions;
			action->execute(registry, event, target, node.properties);

			for (auto it = graph.links.rbegin(); it != graph.links.rend(); ++it) {
				if (it->fromNodeId == node.id && it->fromPinId == PinId::ExecOutput) pending.push_back(it->toNodeId);
			}
		}
	}
}
}

void EventLinkSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (const auto& event : registry.PollAllEvents<Event::GameplayEvent>()) {
		// 旧EventLinkComponentとの互換を保ちながら、同じイベントをグラフにも配送する。
		if (auto* links = registry.GetComponent<Component::EventLinkComponent>(event.source)) {
			for (const auto& link : links->links) {
				if (link.eventId != event.eventId) continue;
				auto* action = Gameplay::GameplayCatalog::Get().FindAction(link.actionId);
				if (!action) {
					LogWarning("EventLinkSystem: unregistered action id: " + link.actionId);
					continue;
				}
				if (action->requiresTarget && link.targetEntity == ECS::INVALID_ENTITY) {
					LogWarning("EventLinkSystem: action requires a target entity: " + link.actionId);
					continue;
				}
				action->execute(registry, event, link.targetEntity, nlohmann::json::object());
			}
		}

		if (auto* graphComponent = registry.GetComponent<Component::GimmickGraphComponent>(event.source)) {
			if (const GimmickGraph* graph = GetGraph(graphComponent->graphPath)) {
				ExecuteGraph(registry, event, *graph);
			}
		}
	}
}
}

#include "../SystemCatalog.h"
REGISTER_SYSTEM(NoEngine::ECS::EventLinkSystem, "EventLinkSystem", "Gameplay")
