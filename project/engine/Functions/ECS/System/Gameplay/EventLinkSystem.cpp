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

void AppendOutgoingNodes(const GimmickGraph& graph, const std::string& nodeId, std::vector<std::string>& pending) {
	for (auto it = graph.links.rbegin(); it != graph.links.rend(); ++it) {
		if (it->fromNodeId == nodeId && it->fromPinId == PinId::ExecOutput) pending.push_back(it->toNodeId);
	}
}
}

void EventLinkSystem::AdvanceGraphExecution(Registry& registry, GraphExecution& execution) {
	constexpr size_t kMaxActionsPerEvent = 1024;
	const auto& catalog = Gameplay::GameplayCatalog::Get();

	if (execution.waiting) {
		const auto* waitingAction = catalog.FindAction(execution.waitingActionId);
		if (waitingAction && waitingAction->isComplete
			&& !waitingAction->isComplete(registry, execution.waitingTarget, execution.waitingProperties)) return;
		AppendOutgoingNodes(execution.graph, execution.waitingNodeId, execution.pendingNodes);
		execution.waiting = false;
	}

	while (!execution.pendingNodes.empty()) {
		if (execution.executedActions >= kMaxActionsPerEvent) {
			LogWarning("EventLinkSystem: graph execution limit exceeded for " + execution.event.eventId);
			execution.pendingNodes.clear();
			return;
		}
		const std::string nodeId = std::move(execution.pendingNodes.back());
		execution.pendingNodes.pop_back();
		auto nodeIt = std::find_if(execution.graph.nodes.begin(), execution.graph.nodes.end(), [&](const auto& node) {
			return node.id == nodeId;
		});
		if (nodeIt == execution.graph.nodes.end() || nodeIt->kind != Gameplay::GimmickNodeKind::Action) continue;
		const auto* action = catalog.FindAction(nodeIt->typeId);
		if (!action) continue;

		Entity target = INVALID_ENTITY;
		for (const auto& property : action->properties) {
			if (property.type != Gameplay::ActionPropertyType::EntityReference) continue;
			auto valueIt = nodeIt->properties.find(property.id);
			if (valueIt == nodeIt->properties.end() || !valueIt->is_string()) continue;
			target = Editor::FindEntityByName(registry, valueIt->get<std::string>());
			if (target == INVALID_ENTITY && property.required) {
				LogWarning("EventLinkSystem: graph action target entity was not found: " + valueIt->get<std::string>());
			}
			break;
		}
		if (action->requiresTarget && target == INVALID_ENTITY) continue;

		++execution.executedActions;
		action->execute(registry, execution.event, target, nodeIt->properties);
		if (action->isComplete && !action->isComplete(registry, target, nodeIt->properties)) {
			execution.waiting = true;
			execution.waitingNodeId = nodeIt->id;
			execution.waitingActionId = nodeIt->typeId;
			execution.waitingTarget = target;
			execution.waitingProperties = nodeIt->properties;
			return;
		}
		AppendOutgoingNodes(execution.graph, nodeIt->id, execution.pendingNodes);
	}
}

void EventLinkSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto it = graphExecutions_.begin(); it != graphExecutions_.end();) {
		AdvanceGraphExecution(registry, *it);
		if (!it->waiting && it->pendingNodes.empty()) it = graphExecutions_.erase(it);
		else ++it;
	}
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
				for (const auto& eventNode : graph->nodes) {
					if (eventNode.kind != Gameplay::GimmickNodeKind::Event || eventNode.typeId != event.eventId) continue;
					GraphExecution execution;
					execution.graph = *graph;
					execution.event = event;
					AppendOutgoingNodes(execution.graph, eventNode.id, execution.pendingNodes);
					graphExecutions_.push_back(std::move(execution));
					AdvanceGraphExecution(registry, graphExecutions_.back());
				}
			}
		}
	}
}
}

#include "../SystemCatalog.h"
REGISTER_SYSTEM(NoEngine::ECS::EventLinkSystem, "EventLinkSystem", "Gameplay")
