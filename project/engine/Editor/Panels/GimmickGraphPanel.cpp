#include "stdafx.h"
#include "GimmickGraphPanel.h"
#include "engine/Functions/ECS/Component/Common/GimmickGraphComponent.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"
#include "engine/Editor/Commands/EditTag.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_node_editor.h"

namespace NoEngine::Editor {
namespace {
namespace NodeEditor = ax::NodeEditor;

NodeEditor::EditorContext* GetNodeEditorContext() {
	static NodeEditor::EditorContext* context = NodeEditor::CreateEditor();
	return context;
}

uint64_t StableId(const std::string& value) {
	uint64_t hash = 1469598103934665603ull;
	for (unsigned char character : value) {
		hash ^= character;
		hash *= 1099511628211ull;
	}
	return hash == 0 ? 1 : hash;
}

std::string NewId(const Gameplay::GimmickGraph& graph, const char* prefix) {
	uint64_t suffix = graph.nodes.size() + graph.links.size() + 1;
	std::string id;
	do {
		id = std::string(prefix) + "-" + std::to_string(suffix++);
	} while (std::any_of(graph.nodes.begin(), graph.nodes.end(), [&](const auto& node) { return node.id == id; })
		|| std::any_of(graph.links.begin(), graph.links.end(), [&](const auto& link) { return link.id == id; }));
	return id;
}

std::string NodeTitle(const Gameplay::GimmickNode& node) {
	const auto& catalog = Gameplay::GameplayCatalog::Get();
	if (node.kind == Gameplay::GimmickNodeKind::Event) {
		if (const auto* event = catalog.FindEvent(node.typeId)) return event->displayName;
	} else if (const auto* action = catalog.FindAction(node.typeId)) {
		return action->displayName;
	}
	return node.typeId;
}

void DrawActionProperties(ECS::Registry& registry, Gameplay::GimmickNode& node,
	const Gameplay::ActionDescriptor& action, bool& dirty) {
	for (const auto& property : action.properties) {
		ImGui::PushID(property.id.c_str());
		auto it = node.properties.find(property.id);
		if (property.type == Gameplay::ActionPropertyType::Boolean) {
			bool value = it != node.properties.end() && it->is_boolean() ? it->get<bool>() : false;
			if (ImGui::Checkbox(property.displayName.c_str(), &value)) { node.properties[property.id] = value; dirty = true; }
		} else if (property.type == Gameplay::ActionPropertyType::Integer) {
			int value = it != node.properties.end() && it->is_number_integer() ? it->get<int>() : 0;
			if (ImGui::InputInt(property.displayName.c_str(), &value)) { node.properties[property.id] = value; dirty = true; }
		} else if (property.type == Gameplay::ActionPropertyType::Number) {
			float value = it != node.properties.end() && it->is_number() ? it->get<float>() : 0.0f;
			if (ImGui::InputFloat(property.displayName.c_str(), &value)) { node.properties[property.id] = value; dirty = true; }
		} else if (property.type == Gameplay::ActionPropertyType::EntityReference) {
			char value[256] = {};
			if (it != node.properties.end() && it->is_string()) {
				const std::string current = it->get<std::string>();
				strncpy_s(value, current.c_str(), _TRUNCATE);
			}
			ImGui::SetNextItemWidth(180.0f);
			if (ImGui::InputText(property.displayName.c_str(), value, sizeof(value))) {
				node.properties[property.id] = value;
				dirty = true;
			}
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_MULTI")) {
					if (payload->DataSize >= static_cast<int>(sizeof(ECS::Entity))) {
						const ECS::Entity droppedEntity = *static_cast<const ECS::Entity*>(payload->Data);
						if (const auto* tag = registry.GetComponent<Editor::EditTag>(droppedEntity)) {
							node.properties[property.id] = tag->name;
							dirty = true;
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
		} else {
			char value[256] = {};
			if (it != node.properties.end() && it->is_string()) {
				const std::string current = it->get<std::string>();
				strncpy_s(value, current.c_str(), _TRUNCATE);
			}
			if (ImGui::InputText(property.displayName.c_str(), value, sizeof(value))) { node.properties[property.id] = value; dirty = true; }
		}
		ImGui::PopID();
	}
}

uint64_t PinStableId(const std::string& nodeId, const char* pinId) {
	return StableId(nodeId + ":" + pinId);
}
}

bool GimmickGraphPanel::Load(const std::string& path) {
	std::string error;
	auto graph = Gameplay::LoadGimmickGraph(path, &error);
	if (!graph) {
		status_ = "Load failed: " + error;
		loaded_ = false;
		return false;
	}
	graph_ = std::move(*graph);
	activePath_ = path;
	contextNodeId_.clear();
	contextLinkId_.clear();
	loaded_ = true;
	dirty_ = false;
	positionedNodeIds_.clear();
	status_.clear();
	return true;
}

void GimmickGraphPanel::Save() {
	if (!loaded_) return;
	const auto validation = Gameplay::ValidateGimmickGraph(graph_);
	if (validation.HasErrors()) {
		status_ = "Cannot save: graph has validation errors.";
		return;
	}
	std::string error;
	if (!Gameplay::SaveGimmickGraph(activePath_, graph_, &error)) {
		status_ = "Save failed: " + error;
		return;
	}
	dirty_ = false;
	status_ = "Saved.";
}

void GimmickGraphPanel::Draw(ECS::Registry& registry, ECS::Entity selectedEntity) {
	ImGui::Begin("Gimmick Graph");
	ownsKeyboardFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
		|| ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
	if (selectedEntity == ECS::INVALID_ENTITY) {
		ImGui::TextDisabled("Select an entity with GimmickGraphComponent.");
		ImGui::End();
		return;
	}
	auto* component = registry.GetComponent<Component::GimmickGraphComponent>(selectedEntity);
	if (!component || component->graphPath.empty()) {
		ImGui::TextDisabled("Select an entity with GimmickGraphComponent.");
		ImGui::End();
		return;
	}
	if (!loaded_ || activePath_ != component->graphPath) Load(component->graphPath);
	if (!loaded_) {
		ImGui::TextWrapped("%s", status_.c_str());
		ImGui::End();
		return;
	}

	if (ImGui::Button("Save Graph")) Save();
	ImGui::SameLine();
	if (dirty_) ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "Unsaved changes");
	else if (!status_.empty()) ImGui::Text("%s", status_.c_str());

	if (ImGui::BeginCombo("Add node", "Choose event or action...")) {
		ImGui::TextDisabled("Events");
		for (const auto& event : Gameplay::GameplayCatalog::Get().GetEvents()) {
			if (ImGui::Selectable(event.displayName.c_str())) {
				Gameplay::GimmickNode node;
				node.id = NewId(graph_, "event");
				node.kind = Gameplay::GimmickNodeKind::Event;
				node.typeId = event.id;
				node.x = 80.0f + 40.0f * graph_.nodes.size();
				node.y = 80.0f + 40.0f * graph_.nodes.size();
				graph_.nodes.push_back(std::move(node)); dirty_ = true;
			}
		}
		ImGui::Separator();
		ImGui::TextDisabled("Actions");
		for (const auto& action : Gameplay::GameplayCatalog::Get().GetActions()) {
			if (ImGui::Selectable(action.displayName.c_str())) {
				Gameplay::GimmickNode node;
				node.id = NewId(graph_, "action");
				node.kind = Gameplay::GimmickNodeKind::Action;
				node.typeId = action.id;
				node.x = 300.0f + 40.0f * graph_.nodes.size();
				node.y = 100.0f + 40.0f * graph_.nodes.size();
				node.properties = nlohmann::json::object();
				for (const auto& property : action.properties) {
					if (property.type == Gameplay::ActionPropertyType::Boolean) node.properties[property.id] = false;
					else if (property.type == Gameplay::ActionPropertyType::Integer) node.properties[property.id] = 0;
					else if (property.type == Gameplay::ActionPropertyType::Number) node.properties[property.id] = 0.0;
					else node.properties[property.id] = "";
				}
				graph_.nodes.push_back(std::move(node)); dirty_ = true;
			}
		}
		ImGui::EndCombo();
	}

	const auto context = GetNodeEditorContext();
	NodeEditor::SetCurrentEditor(context);
	NodeEditor::Begin("GimmickGraphCanvas", ImVec2(0.0f, 0.0f));
	for (size_t i = 0; i < graph_.nodes.size(); ++i) {
		auto& node = graph_.nodes[i];
		const NodeEditor::NodeId nodeEditorId(StableId(node.id));
		if (positionedNodeIds_.insert(node.id).second) NodeEditor::SetNodePosition(nodeEditorId, ImVec2(node.x, node.y));
		ImGui::PushID(node.id.c_str());
		NodeEditor::BeginNode(nodeEditorId);
		ImGui::TextUnformatted(NodeTitle(node).c_str());
		ImGui::Separator();
		if (node.kind == Gameplay::GimmickNodeKind::Action) {
			NodeEditor::BeginPin(NodeEditor::PinId(PinStableId(node.id, Gameplay::PinId::ExecInput)), NodeEditor::PinKind::Input);
			ImGui::Text("In");
			NodeEditor::EndPin();
		}
		if (node.kind == Gameplay::GimmickNodeKind::Action) {
			if (const auto* action = Gameplay::GameplayCatalog::Get().FindAction(node.typeId))
				DrawActionProperties(registry, node, *action, dirty_);
		}
		NodeEditor::BeginPin(NodeEditor::PinId(PinStableId(node.id, Gameplay::PinId::ExecOutput)), NodeEditor::PinKind::Output);
		ImGui::Text("Out");
		NodeEditor::EndPin();
		NodeEditor::EndNode();
		ImGui::PopID();
		const ImVec2 position = NodeEditor::GetNodePosition(nodeEditorId);
		if (position.x != node.x || position.y != node.y) { node.x = position.x; node.y = position.y; dirty_ = true; }
	}
	for (size_t i = 0; i < graph_.links.size(); ++i) {
		const auto& link = graph_.links[i];
		NodeEditor::Link(NodeEditor::LinkId(StableId(link.id)),
			NodeEditor::PinId(PinStableId(link.fromNodeId, link.fromPinId.c_str())),
			NodeEditor::PinId(PinStableId(link.toNodeId, link.toPinId.c_str())));
	}
	const bool creatingLink = NodeEditor::BeginCreate();
	if (creatingLink) {
		NodeEditor::PinId start, end;
		if (NodeEditor::QueryNewLink(&start, &end)) {
			const uint64_t startId = static_cast<uint64_t>(start.Get());
			const uint64_t endId = static_cast<uint64_t>(end.Get());
			const auto findEndpoint = [&](uint64_t pinId, bool wantOutput, std::string& nodeId, std::string& pinName) {
				for (const auto& node : graph_.nodes) {
					const char* candidate = wantOutput ? Gameplay::PinId::ExecOutput : Gameplay::PinId::ExecInput;
					if (PinStableId(node.id, candidate) == pinId) { nodeId = node.id; pinName = candidate; return true; }
				}
				return false;
			};
			Gameplay::GimmickLink link;
			const bool valid = (findEndpoint(startId, true, link.fromNodeId, link.fromPinId)
				&& findEndpoint(endId, false, link.toNodeId, link.toPinId))
				|| (findEndpoint(endId, true, link.fromNodeId, link.fromPinId)
					&& findEndpoint(startId, false, link.toNodeId, link.toPinId));
			if (valid) {
				link.id = NewId(graph_, "link");
				graph_.links.push_back(link);
				if (!Gameplay::ValidateGimmickGraph(graph_).HasErrors()) { NodeEditor::AcceptNewItem(); dirty_ = true; }
				else { graph_.links.pop_back(); NodeEditor::RejectNewItem(); }
			} else NodeEditor::RejectNewItem();
		}
	}
	// BeginCreateは操作がないフレームでも内部状態を開始するため、戻り値がfalseでも必ず閉じる。
	NodeEditor::EndCreate();
	if (NodeEditor::BeginDelete()) {
		NodeEditor::NodeId nodeId;
		while (NodeEditor::QueryDeletedNode(&nodeId)) {
			const auto id = static_cast<uint64_t>(nodeId.Get());
			auto nodeIt = std::find_if(graph_.nodes.begin(), graph_.nodes.end(), [&](const auto& node) { return StableId(node.id) == id; });
			if (nodeIt != graph_.nodes.end()) {
				const std::string stableNodeId = nodeIt->id;
				graph_.links.erase(std::remove_if(graph_.links.begin(), graph_.links.end(), [&](const auto& link) {
					return link.fromNodeId == stableNodeId || link.toNodeId == stableNodeId;
				}), graph_.links.end());
				graph_.nodes.erase(nodeIt);
				NodeEditor::AcceptDeletedItem();
				dirty_ = true;
			}
		}
		NodeEditor::LinkId linkId;
		while (NodeEditor::QueryDeletedLink(&linkId)) {
			const auto id = static_cast<uint64_t>(linkId.Get());
			auto it = std::find_if(graph_.links.begin(), graph_.links.end(), [&](const auto& link) { return StableId(link.id) == id; });
			if (it != graph_.links.end()) { graph_.links.erase(it); NodeEditor::AcceptDeletedItem(); dirty_ = true; }
		}
		NodeEditor::EndDelete();
	}
	bool openNodeContextPopup = false;
	bool openLinkContextPopup = false;
	NodeEditor::NodeId contextNode;
	if (NodeEditor::ShowNodeContextMenu(&contextNode)) {
		for (const auto& node : graph_.nodes) {
			if (StableId(node.id) == static_cast<uint64_t>(contextNode.Get())) {
				contextNodeId_ = node.id;
				openNodeContextPopup = true;
				break;
			}
		}
	}
	NodeEditor::LinkId contextLink;
	if (NodeEditor::ShowLinkContextMenu(&contextLink)) {
		for (const auto& link : graph_.links) {
			if (StableId(link.id) == static_cast<uint64_t>(contextLink.Get())) {
				contextLinkId_ = link.id;
				openLinkContextPopup = true;
				break;
			}
		}
	}
	// ShowNode/LinkContextMenuは描画チャネルを操作するため、必ずNodeEditor::End()前に呼ぶ。
	NodeEditor::End();
	NodeEditor::SetCurrentEditor(nullptr);

	if (openNodeContextPopup) ImGui::OpenPopup("Gimmick Graph Node Context");
	if (openLinkContextPopup) ImGui::OpenPopup("Gimmick Graph Link Context");
	if (ImGui::BeginPopup("Gimmick Graph Node Context")) {
		if (ImGui::MenuItem("Delete Node")) {
			graph_.links.erase(std::remove_if(graph_.links.begin(), graph_.links.end(), [&](const auto& link) {
				return link.fromNodeId == contextNodeId_ || link.toNodeId == contextNodeId_;
			}), graph_.links.end());
			graph_.nodes.erase(std::remove_if(graph_.nodes.begin(), graph_.nodes.end(), [&](const auto& node) {
				return node.id == contextNodeId_;
			}), graph_.nodes.end());
			positionedNodeIds_.erase(contextNodeId_);
			dirty_ = true;
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopup("Gimmick Graph Link Context")) {
		if (ImGui::MenuItem("Delete Link")) {
			graph_.links.erase(std::remove_if(graph_.links.begin(), graph_.links.end(), [&](const auto& link) {
				return link.id == contextLinkId_;
			}), graph_.links.end());
			dirty_ = true;
		}
		ImGui::EndPopup();
	}

	const auto validation = Gameplay::ValidateGimmickGraph(graph_);
	for (const auto& diagnostic : validation.diagnostics) {
		if (diagnostic.severity == Gameplay::DiagnosticSeverity::Error) ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", diagnostic.message.c_str());
	}
	ImGui::End();
}

}
