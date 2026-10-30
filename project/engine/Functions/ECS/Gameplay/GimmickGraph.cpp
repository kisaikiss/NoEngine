#include "stdafx.h"
#include "GimmickGraph.h"

namespace NoEngine::Gameplay {
namespace {
using json = nlohmann::json;

const char* KindToString(GimmickNodeKind kind) {
	return kind == GimmickNodeKind::Event ? "event" : "action";
}

bool TryParseKind(const std::string& value, GimmickNodeKind& kind) {
	if (value == "event") { kind = GimmickNodeKind::Event; return true; }
	if (value == "action") { kind = GimmickNodeKind::Action; return true; }
	return false;
}

void SetError(std::string* error, const std::string& message) {
	if (error) *error = message;
}

bool IsPropertyType(const json& value, ActionPropertyType type) {
	switch (type) {
	case ActionPropertyType::Boolean: return value.is_boolean();
	case ActionPropertyType::Integer: return value.is_number_integer() || value.is_number_unsigned();
	case ActionPropertyType::Number: return value.is_number();
	case ActionPropertyType::String:
	case ActionPropertyType::EntityReference: return value.is_string();
	}
	return false;
}

void AddDiagnostic(GimmickGraphValidation& result, DiagnosticSeverity severity,
	std::string code, std::string message, std::string nodeId = {}, std::string linkId = {}) {
	result.diagnostics.push_back({ severity, std::move(code), std::move(message), std::move(nodeId), std::move(linkId) });
}
}

json GimmickGraph::ToJson() const {
	json output;
	output["version"] = version;
	output["nodes"] = json::array();
	for (const auto& node : nodes) {
		output["nodes"].push_back({
			{ "id", node.id },
			{ "kind", KindToString(node.kind) },
			{ "typeId", node.typeId },
			{ "position", { node.x, node.y } },
			{ "properties", node.properties }
		});
	}
	output["links"] = json::array();
	for (const auto& link : links) {
		output["links"].push_back({
			{ "id", link.id },
			{ "fromNodeId", link.fromNodeId },
			{ "fromPinId", link.fromPinId },
			{ "toNodeId", link.toNodeId },
			{ "toPinId", link.toPinId }
		});
	}
	return output;
}

std::optional<GimmickGraph> GimmickGraph::FromJson(const json& input, std::string* error) {
	try {
		if (!input.is_object() || !input.contains("nodes") || !input["nodes"].is_array()
			|| !input.contains("links") || !input["links"].is_array()) {
			SetError(error, "Graph must contain nodes and links arrays.");
			return std::nullopt;
		}

		GimmickGraph graph;
		graph.version = input.value("version", 0u);
		for (const auto& item : input["nodes"]) {
			if (!item.is_object() || !item.contains("kind") || !item["kind"].is_string()) {
				SetError(error, "Each node must have a string kind.");
				return std::nullopt;
			}
			GimmickNode node;
			std::string kindName = item["kind"].get<std::string>();
			if (!TryParseKind(kindName, node.kind)) {
				SetError(error, "Unknown node kind: " + kindName);
				return std::nullopt;
			}
			node.id = item.value("id", std::string{});
			node.typeId = item.value("typeId", std::string{});
			if (item.contains("position")) {
				if (!item["position"].is_array() || item["position"].size() != 2
					|| !item["position"][0].is_number() || !item["position"][1].is_number()) {
					SetError(error, "Node position must be a pair of numbers.");
					return std::nullopt;
				}
				node.x = item["position"][0].get<float>();
				node.y = item["position"][1].get<float>();
			}
			node.properties = item.value("properties", json::object());
			graph.nodes.push_back(std::move(node));
		}
		for (const auto& item : input["links"]) {
			if (!item.is_object()) {
				SetError(error, "Each link must be an object.");
				return std::nullopt;
			}
			GimmickLink link;
			link.id = item.value("id", std::string{});
			link.fromNodeId = item.value("fromNodeId", std::string{});
			link.fromPinId = item.value("fromPinId", std::string{});
			link.toNodeId = item.value("toNodeId", std::string{});
			link.toPinId = item.value("toPinId", std::string{});
			graph.links.push_back(std::move(link));
		}
		return graph;
	} catch (const std::exception& exception) {
		SetError(error, std::string("Invalid graph JSON: ") + exception.what());
		return std::nullopt;
	}
}

bool SaveGimmickGraph(const std::filesystem::path& path, const GimmickGraph& graph, std::string* error) {
	try {
		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		if (!file) { SetError(error, "Could not open graph file for writing: " + path.string()); return false; }
		file << graph.ToJson().dump(2);
		if (!file) { SetError(error, "Could not write graph file: " + path.string()); return false; }
		return true;
	} catch (const std::exception& exception) {
		SetError(error, std::string("Could not save graph: ") + exception.what());
		return false;
	}
}

std::optional<GimmickGraph> LoadGimmickGraph(const std::filesystem::path& path, std::string* error) {
	try {
		std::ifstream file(path, std::ios::binary);
		if (!file) { SetError(error, "Could not open graph file: " + path.string()); return std::nullopt; }
		json input;
		file >> input;
		return GimmickGraph::FromJson(input, error);
	} catch (const std::exception& exception) {
		SetError(error, std::string("Could not load graph: ") + exception.what());
		return std::nullopt;
	}
}

bool GimmickGraphValidation::HasErrors() const {
	return std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
		return diagnostic.severity == DiagnosticSeverity::Error;
	});
}

GimmickGraphValidation ValidateGimmickGraph(const GimmickGraph& graph, const GameplayCatalog& catalog) {
	GimmickGraphValidation result;
	if (graph.version != kGimmickGraphVersion) {
		AddDiagnostic(result, DiagnosticSeverity::Error, "graph.version.unsupported",
			"Unsupported graph version " + std::to_string(graph.version) + ".");
	}

	std::unordered_map<std::string, const GimmickNode*> nodesById;
	std::unordered_map<std::string, const ActionDescriptor*> actionsByNode;
	std::unordered_map<std::string, std::vector<std::string>> adjacency;
	std::unordered_set<std::string> linkIds;
	std::unordered_set<std::string> incomingActionNodes;
	std::unordered_set<std::string> eventNodes;

	for (const auto& node : graph.nodes) {
		if (node.id.empty()) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "node.id.empty", "Node ID must not be empty.");
			continue;
		}
		if (!nodesById.emplace(node.id, &node).second) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "node.id.duplicate", "Duplicate node ID.", node.id);
			continue;
		}
		if (node.typeId.empty()) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "node.type.empty", "Node type ID must not be empty.", node.id);
			continue;
		}
		if (!node.properties.is_object()) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "node.properties.invalid", "Node properties must be a JSON object.", node.id);
			continue;
		}

		if (node.kind == GimmickNodeKind::Event) {
			if (!catalog.FindEvent(node.typeId)) {
				AddDiagnostic(result, DiagnosticSeverity::Error, "node.event.unregistered",
					"Event ID is not registered: " + node.typeId, node.id);
			} else {
				eventNodes.insert(node.id);
			}
			if (!node.properties.empty()) {
				AddDiagnostic(result, DiagnosticSeverity::Warning, "node.event.unused_properties",
					"This event node type does not define configurable properties.", node.id);
			}
		} else {
			const ActionDescriptor* action = catalog.FindAction(node.typeId);
			if (!action) {
				AddDiagnostic(result, DiagnosticSeverity::Error, "node.action.unregistered",
					"Action ID is not registered: " + node.typeId, node.id);
				continue;
			}
			actionsByNode.emplace(node.id, action);
			for (const auto& property : action->properties) {
				auto valueIt = node.properties.find(property.id);
				if (valueIt == node.properties.end()) {
					if (property.required) {
						AddDiagnostic(result, DiagnosticSeverity::Error, "node.property.required",
							"Required property is missing: " + property.id, node.id);
					}
					continue;
				}
				if (!IsPropertyType(*valueIt, property.type)) {
					AddDiagnostic(result, DiagnosticSeverity::Error, "node.property.type",
						"Property has the wrong JSON type: " + property.id, node.id);
				}
			}
			for (auto propertyIt = node.properties.begin(); propertyIt != node.properties.end(); ++propertyIt) {
				const bool known = std::any_of(action->properties.begin(), action->properties.end(), [&](const auto& definition) {
					return definition.id == propertyIt.key();
				});
				if (!known) {
					AddDiagnostic(result, DiagnosticSeverity::Warning, "node.property.unknown",
						"Action does not define property: " + propertyIt.key(), node.id);
				}
			}
		}
	}

	if (eventNodes.empty()) {
		AddDiagnostic(result, DiagnosticSeverity::Warning, "graph.event.none", "Graph has no valid event source node.");
	}

	for (const auto& link : graph.links) {
		if (link.id.empty()) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "link.id.empty", "Link ID must not be empty.", {}, link.id);
		} else if (!linkIds.insert(link.id).second) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "link.id.duplicate", "Duplicate link ID.", {}, link.id);
		}
		auto fromIt = nodesById.find(link.fromNodeId);
		auto toIt = nodesById.find(link.toNodeId);
		if (fromIt == nodesById.end() || toIt == nodesById.end()) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "link.node.missing", "Link refers to a missing node.", {}, link.id);
			continue;
		}
		const GimmickNode& from = *fromIt->second;
		const GimmickNode& to = *toIt->second;
		const bool validFrom = link.fromPinId == PinId::ExecOutput;
		const bool validTo = link.toPinId == PinId::ExecInput && to.kind == GimmickNodeKind::Action;
		if (!validFrom || !validTo) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "link.pin.invalid",
				"Execution links must connect exec.out to an action's exec.in.", {}, link.id);
			continue;
		}
		if (from.kind != GimmickNodeKind::Event && from.kind != GimmickNodeKind::Action) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "link.source.invalid", "Invalid link source node.", from.id, link.id);
			continue;
		}
		if (!incomingActionNodes.insert(to.id).second) {
			AddDiagnostic(result, DiagnosticSeverity::Error, "link.input.multiple",
				"An action execution input can have only one incoming link.", to.id, link.id);
		}
		adjacency[from.id].push_back(to.id);
	}

	// 最初のグラフ形式ではサイクルは許容されませんが、明示的なループノードを用いることで、後からこれを緩和することが可能です。
	std::unordered_map<std::string, uint8_t> visitState;
	std::function<void(const std::string&)> visit = [&](const std::string& nodeId) {
		visitState[nodeId] = 1;
		for (const auto& next : adjacency[nodeId]) {
			if (visitState[next] == 1) {
				AddDiagnostic(result, DiagnosticSeverity::Error, "graph.cycle", "Execution graph contains a cycle.", next);
			} else if (visitState[next] == 0) {
				visit(next);
			}
		}
		visitState[nodeId] = 2;
	};
	for (const auto& [nodeId, node] : nodesById) {
		static_cast<void>(node);
		if (visitState[nodeId] == 0) visit(nodeId);
	}

	std::unordered_set<std::string> reachable;
	std::queue<std::string> pending;
	for (const auto& eventNodeId : eventNodes) pending.push(eventNodeId);
	while (!pending.empty()) {
		auto current = std::move(pending.front());
		pending.pop();
		if (!reachable.insert(current).second) continue;
		for (const auto& next : adjacency[current]) pending.push(next);
	}
	for (const auto& [nodeId, action] : actionsByNode) {
		static_cast<void>(action);
		if (!reachable.contains(nodeId)) {
			AddDiagnostic(result, DiagnosticSeverity::Warning, "node.action.unreachable",
				"Action node cannot be reached from an event source.", nodeId);
		}
	}
	return result;
}

}
