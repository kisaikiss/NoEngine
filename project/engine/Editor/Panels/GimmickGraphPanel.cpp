#include "stdafx.h"
#include "GimmickGraphPanel.h"
#ifdef USE_IMGUI
#include "engine/Functions/ECS/Component/Common/GimmickGraphComponent.h"
#include "engine/Functions/ECS/Gameplay/GameplayCatalog.h"
#include "engine/Editor/Commands/EditTag.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Functions/Command/EditCommand/FunctionCommand.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_node_editor.h"
#include <commdlg.h>

#pragma comment(lib, "Comdlg32.lib")

namespace NoEngine::Editor {
namespace {
namespace NodeEditor = ax::NodeEditor;

NodeEditor::EditorContext* GetNodeEditorContext() {
	// Canvasのズーム・選択・ノード位置などimgui-node-editor固有の状態を保持する。
	static NodeEditor::EditorContext* context = NodeEditor::CreateEditor();
	return context;
}

std::unordered_map<std::string, std::shared_ptr<GimmickGraphEditorDocument>>& GetGraphDocuments() {
	// パネルを閉じたり別Entityを選んだりしても未保存編集とUndo対象を保持する。
	static std::unordered_map<std::string, std::shared_ptr<GimmickGraphEditorDocument>> documents;
	return documents;
}

std::string NormalizeGraphPath(const std::string& path) {
	// "a/../b"のような異なる表記で同じDocumentが二重に作られないようにする。
	return std::filesystem::path(path).lexically_normal().generic_string();
}

std::optional<std::filesystem::path> ChooseGraphFile(bool save) {
	// Windows標準ダイアログで既存Graphを選ぶか、新規Graphの保存先を指定する。
	std::array<char, 32768> pathBuffer{};
	const char filter[] = "Gimmick Graph (*.json)\0*.json\0JSON files (*.json)\0*.json\0All files (*.*)\0*.*\0\0";
	const auto currentDirectory = std::filesystem::current_path().string();
	OPENFILENAMEA dialog{};
	dialog.lStructSize = sizeof(dialog);
	dialog.lpstrFilter = filter;
	dialog.lpstrFile = pathBuffer.data();
	dialog.nMaxFile = static_cast<DWORD>(pathBuffer.size());
	dialog.lpstrInitialDir = currentDirectory.c_str();
	dialog.lpstrDefExt = "json";
	dialog.lpstrTitle = save ? "Create Gimmick Graph" : "Open Gimmick Graph";
	// ファイルダイアログの移動先でアプリの相対パス基準が変わらないようにする。
	dialog.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR
		| (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
	const BOOL accepted = save ? GetSaveFileNameA(&dialog) : GetOpenFileNameA(&dialog);
	// Windowsの設定や環境によって作業ディレクトリが変わる場合にも元の場所へ戻す。
	std::error_code restoreError;
	std::filesystem::current_path(currentDirectory, restoreError);
	if (!accepted) return std::nullopt;
	return std::filesystem::path(pathBuffer.data()).lexically_normal();
}

std::string ToStoredGraphPath(const std::filesystem::path& path) {
	// プロジェクトの作業ディレクトリ内なら相対パスにし、シーンデータを持ち運びやすくする。
	std::error_code error;
	const auto relativePath = std::filesystem::relative(path, std::filesystem::current_path(), error);
	return (error || relativePath.empty() ? path : relativePath).generic_string();
}

void AddGraphSnapshotCommand(const std::shared_ptr<GimmickGraphEditorDocument>& document,
	const Gameplay::GimmickGraph& before, const Gameplay::GimmickGraph& after) {
	// Undo/Redo履歴には編集前後のGraph全体を保存し、UIクラスの寿命に依存させない。
	auto oldGraph = before;
	auto newGraph = after;
	EditorCommandOperator::AddCommand(std::make_unique<Command::FunctionCommand>(
		[document, newGraph]() { document->graph = newGraph; document->dirty = true; ++document->revision; },
		[document, oldGraph]() { document->graph = oldGraph; document->dirty = true; ++document->revision; },
		"Edit Gimmick Graph"));
}

uint64_t StableId(const std::string& value) {
	// NodeEditorは整数IDを要求するため、保存文字列IDを決定的な64bit値へ変換する。
	// JSONには元の文字列IDを保存し、ハッシュ値はUIの内部識別にだけ使う。
	uint64_t hash = 1469598103934665603ull;
	for (unsigned char character : value) {
		hash ^= character;
		hash *= 1099511628211ull;
	}
	return hash == 0 ? 1 : hash;
}

std::string NewId(const Gameplay::GimmickGraph& graph, const char* prefix) {
	// ノードIDとリンクIDのどちらとも衝突しないIDを生成する。
	uint64_t suffix = graph.nodes.size() + graph.links.size() + 1;
	std::string id;
	do {
		id = std::string(prefix) + "-" + std::to_string(suffix++);
	} while (std::any_of(graph.nodes.begin(), graph.nodes.end(), [&](const auto& node) { return node.id == id; })
		|| std::any_of(graph.links.begin(), graph.links.end(), [&](const auto& link) { return link.id == id; }));
	return id;
}

std::string NodeTitle(const Gameplay::GimmickNode& node) {
	// 保存データのtypeIdは安定IDのままにして、画面にはカタログの表示名を出す。
	const auto& catalog = Gameplay::GameplayCatalog::Get();
	if (node.kind == Gameplay::GimmickNodeKind::Event) {
		if (const auto* event = catalog.FindEvent(node.typeId)) return event->displayName;
	} else if (const auto* action = catalog.FindAction(node.typeId)) {
		return action->displayName;
	}
	return node.typeId;
}

void DrawActionProperties(ECS::Registry& registry, Gameplay::GimmickNode& node,
	const Gameplay::ActionDescriptor& action, bool& dirty, bool& valueInputActive) {
	// 入力欄はActionDescriptorから生成するため、新Actionを登録すると対応UIも自動で増える。
	for (const auto& property : action.properties) {
		// 同じ名前のプロパティを持つノードが複数あっても、ImGuiのWidget IDを混同しない。
		ImGui::PushID(property.id.c_str());
		auto it = node.properties.find(property.id);
		if (property.type == Gameplay::ActionPropertyType::Boolean) {
			bool value = it != node.properties.end() && it->is_boolean() ? it->get<bool>() : false;
			if (ImGui::Checkbox(property.displayName.c_str(), &value)) { node.properties[property.id] = value; dirty = true; }
		} else if (property.type == Gameplay::ActionPropertyType::Integer) {
			const int currentValue = it != node.properties.end() && it->is_number_integer() ? it->get<int>() : 0;
			char value[64] = {};
			_snprintf_s(value, sizeof(value), _TRUNCATE, "%d", currentValue);
			// InputIntはEnterReturnsTrueを禁止しているため、文字列欄で編集しEnter時に数値化する。
			if (ImGui::InputText(property.displayName.c_str(), value, sizeof(value), ImGuiInputTextFlags_EnterReturnsTrue)) {
				char* end = nullptr;
				const long parsedValue = std::strtol(value, &end, 10);
				if (end != value && *end == '\0' && parsedValue >= INT_MIN && parsedValue <= INT_MAX) {
					node.properties[property.id] = static_cast<int>(parsedValue);
					dirty = true;
				}
			}
		} else if (property.type == Gameplay::ActionPropertyType::Number) {
			float value = it != node.properties.end() && it->is_number() ? it->get<float>() : 0.0f;
			ImGui::SetNextItemWidth(180.0f);
			if (ImGui::DragFloat(property.displayName.c_str(), &value, 0.1f, 0.0f, 0.0f, "%.3f")) {
				node.properties[property.id] = value;
				dirty = true;
			}
			valueInputActive = valueInputActive || ImGui::IsItemActive();
		} else if (property.type == Gameplay::ActionPropertyType::String) {
			char value[256] = {};
			if (it != node.properties.end() && it->is_string()) {
				const std::string current = it->get<std::string>();
				strncpy_s(value, current.c_str(), _TRUNCATE);
			}
			ImGui::SetNextItemWidth(180.0f);
			if (ImGui::InputText(property.displayName.c_str(), value, sizeof(value), ImGuiInputTextFlags_EnterReturnsTrue)) {
				node.properties[property.id] = value;
				dirty = true;
			}
		} else if (property.type == Gameplay::ActionPropertyType::EntityReference) {
			// JSONにはEntityハンドルではなく名前を保存する。シーンを再ロードしても参照を解決できるため。
			char value[256] = {};
			if (it != node.properties.end() && it->is_string()) {
				const std::string current = it->get<std::string>();
				strncpy_s(value, current.c_str(), _TRUNCATE);
			}
			ImGui::SetNextItemWidth(180.0f);
			if (ImGui::InputText(property.displayName.c_str(), value, sizeof(value), ImGuiInputTextFlags_EnterReturnsTrue)) {
				node.properties[property.id] = value;
				dirty = true;
			}
			// HierarchyのENTITY_MULTI payloadは先頭Entityを代表として持つため、そのEditTag名を保存する。
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
			if (ImGui::InputText(property.displayName.c_str(), value, sizeof(value), ImGuiInputTextFlags_EnterReturnsTrue)) { node.properties[property.id] = value; dirty = true; }
		}
		ImGui::PopID();
	}
}

uint64_t PinStableId(const std::string& nodeId, const char* pinId) {
	// 同じ種類のピンでもノードごとに一意なIDになるよう、Node IDとPin IDを組み合わせる。
	return StableId(nodeId + ":" + pinId);
}
}

bool GimmickGraphPanel::Load(const std::string& path) {
	// UIが参照するグラフはファイルそのものではなく、Undo可能なEditorDocumentとして保持する。
	// 同じファイルを既に編集していればメモリ上のDocumentを再利用し、未保存内容を保つ。
	const std::string normalizedPath = NormalizeGraphPath(path);
	if (auto documentIt = GetGraphDocuments().find(normalizedPath); documentIt != GetGraphDocuments().end()) {
		document_ = documentIt->second;
		activePath_ = path;
		loaded_ = true;
		observedDocumentRevision_ = document_->revision;
		pendingMouseEditStart_.reset();
		contextNodeId_.clear();
		contextLinkId_.clear();
		positionedNodeIds_.clear();
		status_.clear();
		return true;
	}
	std::string error;
	auto graph = Gameplay::LoadGimmickGraph(path, &error);
	if (!graph) {
		activePath_ = path;
		document_.reset();
		pendingMouseEditStart_.reset();
		positionedNodeIds_.clear();
		status_ = "Load failed: " + error;
		loaded_ = false;
		return false;
	}
	document_ = std::make_shared<GimmickGraphEditorDocument>();
	document_->graph = std::move(*graph);
	activePath_ = path;
	observedDocumentRevision_ = document_->revision;
	pendingMouseEditStart_.reset();
	GetGraphDocuments().emplace(normalizedPath, document_);
	contextNodeId_.clear();
	contextLinkId_.clear();
	loaded_ = true;
	positionedNodeIds_.clear();
	status_.clear();
	return true;
}

void GimmickGraphPanel::Save() {
	// 保存操作は明示的に行う。編集データを先に検証し、問題があればファイルを更新しない。
	if (!loaded_ || !document_) return;
	// エラーのあるデータは保存せず、エディタ上で直してから再試行させる。
	const auto validation = Gameplay::ValidateGimmickGraph(document_->graph);
	if (validation.HasErrors()) {
		status_ = "Cannot save: graph has validation errors.";
		return;
	}
	std::string error;
	if (!Gameplay::SaveGimmickGraph(activePath_, document_->graph, &error)) {
		status_ = "Save failed: " + error;
		return;
	}
	document_->dirty = false;
	status_ = "Saved.";
}

void GimmickGraphPanel::Draw(ECS::Registry& registry, ECS::Entity selectedEntity) {
	ImGui::Begin("Gimmick Graph");
	ownsKeyboardFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
		|| ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
	// グラフの編集対象はHierarchy/Inspectorと同じ選択Entityから決める。
	if (selectedEntity == ECS::INVALID_ENTITY) {
		ImGui::TextDisabled("Select an entity with GimmickGraphComponent.");
		ImGui::End();
		return;
	}
	auto* component = registry.GetComponent<Component::GimmickGraphComponent>(selectedEntity);
	// Graphファイルの場所は選択中EntityのComponentが指す。別EntityならそのDocumentへ切り替える。
	if (!component) {
		ImGui::TextDisabled("Select an entity with GimmickGraphComponent.");
		ImGui::End();
		return;
	}
	if (component->graphPath.empty()) {
		activePath_.clear();
		document_.reset();
		loaded_ = false;
		pendingMouseEditStart_.reset();
		positionedNodeIds_.clear();
		status_ = "No Gimmick Graph is assigned to this entity.";
	} else if (activePath_ != component->graphPath) {
		Load(component->graphPath);
	}
	if (!loaded_) {
		ImGui::TextWrapped("%s", status_.c_str());
		if (ImGui::Button("Open")) {
			if (const auto selectedPath = ChooseGraphFile(false)) {
				component->graphPath = ToStoredGraphPath(*selectedPath);
				Load(component->graphPath);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("New")) {
			if (const auto selectedPath = ChooseGraphFile(true)) {
				Gameplay::GimmickGraph newGraph;
				std::string error;
				if (Gameplay::SaveGimmickGraph(*selectedPath, newGraph, &error)) {
					component->graphPath = ToStoredGraphPath(*selectedPath);
					Load(component->graphPath);
				} else {
					status_ = "Create failed: " + error;
				}
			}
		}
		ImGui::End();
		return;
	}
	auto& graph_ = document_->graph;
	bool& dirty_ = document_->dirty;
	// Undo/RedoでGraphが差し替わったらNodeEditor内部の位置キャッシュをJSONの座標へ同期し直す。
	if (observedDocumentRevision_ != document_->revision) {
		positionedNodeIds_.clear();
		observedDocumentRevision_ = document_->revision;
	}

	if (ImGui::Button("Save Graph")) Save();
	ImGui::SameLine();
	// Scene編集と共通のUndo履歴を使うため、グラフ操作と他のエディタ操作を一つの履歴で戻せる。
	if (ImGui::Button("Undo")) EditorCommandOperator::Undo();
	ImGui::SameLine();
	if (ImGui::Button("Redo")) EditorCommandOperator::Redo();
	ImGui::SameLine();
	if (dirty_) ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "Unsaved changes");
	else if (!status_.empty()) ImGui::Text("%s", status_.c_str());

	// このフレーム中の操作をひとつの差分として比較し、既存Undo履歴へ積む。
	const Gameplay::GimmickGraph graphBeforeFrame = graph_;
	bool valueInputActive = false;
	if (ImGui::BeginCombo("Add node", "Choose event or action...")) {
		// ノード一覧はカタログを直接参照する。アプリがActionを登録するとこの一覧にも現れる。
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
	NodeEditor::PushStyleVar(NodeEditor::StyleVar_PinRounding, 5.0f);
	NodeEditor::PushStyleVar(NodeEditor::StyleVar_PinBorderWidth, 1.5f);
	NodeEditor::PushStyleVar(NodeEditor::StyleVar_PinRadius, 5.0f);
	NodeEditor::PushStyleColor(NodeEditor::StyleColor_PinRect, ImVec4(0.25f, 0.72f, 1.0f, 0.45f));
	NodeEditor::PushStyleColor(NodeEditor::StyleColor_PinRectBorder, ImVec4(0.55f, 0.85f, 1.0f, 0.9f));
	// 保存IDからNodeEditor IDを作り、表示・プロパティ編集・保存座標の同期を行う。
	for (size_t i = 0; i < graph_.nodes.size(); ++i) {
		auto& node = graph_.nodes[i];
		const NodeEditor::NodeId nodeEditorId(StableId(node.id));
		// JSON座標はDocument切替時やUndo/Redo後だけUIへ適用し、毎フレームのドラッグを上書きしない。
		if (positionedNodeIds_.insert(node.id).second) NodeEditor::SetNodePosition(nodeEditorId, ImVec2(node.x, node.y));
		ImGui::PushID(node.id.c_str());
		NodeEditor::BeginNode(nodeEditorId);
		ImGui::TextUnformatted(NodeTitle(node).c_str());
		// SeparatorはNodeEditor内のウィンドウ幅まで伸びるため、ノード内容幅に合わせて線を描く。
		const ImVec2 dividerStart = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(dividerStart.x, dividerStart.y + 3.0f),
			ImVec2(dividerStart.x + 180.0f, dividerStart.y + 3.0f),
			IM_COL32(110, 110, 110, 255), 1.0f);
		ImGui::Dummy(ImVec2(180.0f, 8.0f));
		if (node.kind == Gameplay::GimmickNodeKind::Action) {
			// 入力ピンの当たり判定と配線の接続点を文字ラベルではなく丸印に限定する。
			NodeEditor::BeginPin(NodeEditor::PinId(PinStableId(node.id, Gameplay::PinId::ExecInput)), NodeEditor::PinKind::Input);
			ImGui::Dummy(ImVec2(10.0f, 10.0f));
			const ImVec2 inputPinMin = ImGui::GetItemRectMin();
			const ImVec2 inputPinMax = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2((inputPinMin.x + inputPinMax.x) / 2.0f, (inputPinMin.y + inputPinMax.y) / 2.0f), 4.0f, IM_COL32(90, 205, 255, 255));
			ImGui::SameLine(0.0f, 4.0f);
			ImGui::TextUnformatted("In");
			NodeEditor::PinRect(inputPinMin, inputPinMax);
			NodeEditor::PinPivotRect(inputPinMin, inputPinMax);
			NodeEditor::EndPin();
		}
		if (node.kind == Gameplay::GimmickNodeKind::Action) {
			if (const auto* action = Gameplay::GameplayCatalog::Get().FindAction(node.typeId))
				DrawActionProperties(registry, node, *action, dirty_, valueInputActive);
		}
		NodeEditor::BeginPin(NodeEditor::PinId(PinStableId(node.id, Gameplay::PinId::ExecOutput)), NodeEditor::PinKind::Output);
		// 前フレームに計測されたノード幅から出力を右寄せする。丸印をPinPivotにして線をそこへ接続する。
		const auto& nodeStyle = NodeEditor::GetStyle();
		const float nodeWidth = NodeEditor::GetNodeSize(nodeEditorId).x;
		const float outputLabelWidth = ImGui::CalcTextSize("Out").x + 14.0f;
		const float rightOffset = std::max(120.0f,
			nodeWidth - nodeStyle.NodePadding.x - nodeStyle.NodePadding.z - outputLabelWidth);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + rightOffset);
		ImGui::TextUnformatted("Out");
		ImGui::SameLine(0.0f, 4.0f);
		ImGui::Dummy(ImVec2(10.0f, 10.0f));
		const ImVec2 outputPinMin = ImGui::GetItemRectMin();
		const ImVec2 outputPinMax = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2((outputPinMin.x + outputPinMax.x) / 2.0f, (outputPinMin.y + outputPinMax.y) / 2.0f), 4.0f, IM_COL32(255, 174, 64, 255));
		NodeEditor::PinRect(outputPinMin, outputPinMax);
		NodeEditor::PinPivotRect(outputPinMin, outputPinMax);
		NodeEditor::EndPin();
		NodeEditor::EndNode();
		ImGui::PopID();
		// NodeEditor内で動かされた位置をDocumentへ戻す。これが次回JSON保存される座標になる。
		const ImVec2 position = NodeEditor::GetNodePosition(nodeEditorId);
		if (position.x != node.x || position.y != node.y) { node.x = position.x; node.y = position.y; dirty_ = true; }
	}
	NodeEditor::PopStyleColor(2);
	NodeEditor::PopStyleVar(3);
	for (size_t i = 0; i < graph_.links.size(); ++i) {
		// 保存リンクの文字列IDをUI用IDへ変換し、対応する2つのPinを描画する。
		const auto& link = graph_.links[i];
		NodeEditor::Link(NodeEditor::LinkId(StableId(link.id)),
			NodeEditor::PinId(PinStableId(link.fromNodeId, link.fromPinId.c_str())),
			NodeEditor::PinId(PinStableId(link.toNodeId, link.toPinId.c_str())));
	}
	// ユーザーのドラッグ接続をGraphLinkへ変換し、共通Validatorに通る場合だけ確定する。
	const bool creatingLink = NodeEditor::BeginCreate();
	if (creatingLink) {
		NodeEditor::PinId start, end;
		if (NodeEditor::QueryNewLink(&start, &end)) {
			const uint64_t startId = static_cast<uint64_t>(start.Get());
			const uint64_t endId = static_cast<uint64_t>(end.Get());
			// マウス操作で返るPin IDから、保存形式で使うNode ID／exec.in・exec.outへ戻す。
			const auto findEndpoint = [&](uint64_t pinId, bool wantOutput, std::string& nodeId, std::string& pinName) {
				for (const auto& node : graph_.nodes) {
					const char* candidate = wantOutput ? Gameplay::PinId::ExecOutput : Gameplay::PinId::ExecInput;
					if (PinStableId(node.id, candidate) == pinId) { nodeId = node.id; pinName = candidate; return true; }
				}
				return false;
				};
			Gameplay::GimmickLink link;
			// ユーザーはどちら側からドラッグしてもよいが、保存時は必ず出力→入力に正規化する。
			const bool valid = (findEndpoint(startId, true, link.fromNodeId, link.fromPinId)
				&& findEndpoint(endId, false, link.toNodeId, link.toPinId))
				|| (findEndpoint(endId, true, link.fromNodeId, link.fromPinId)
					&& findEndpoint(startId, false, link.toNodeId, link.toPinId));
			if (valid) {
				link.id = NewId(graph_, "link");
				graph_.links.push_back(link);
				// 追加後のGraph全体が有効な場合だけUI操作を確定し、不正なら直前の追加を取り消す。
				if (!Gameplay::ValidateGimmickGraph(graph_).HasErrors()) { NodeEditor::AcceptNewItem(); dirty_ = true; } else { graph_.links.pop_back(); NodeEditor::RejectNewItem(); }
			} else NodeEditor::RejectNewItem();
		}
	}
	// BeginCreateは操作がないフレームでも内部状態を開始するため、戻り値がfalseでも必ず閉じる。
	NodeEditor::EndCreate();
	// キーボード削除を使う場合のNodeEditor通知も、内部Graph配列に反映する。
	if (NodeEditor::BeginDelete()) {
		// NodeEditorの削除通知を保存データにも反映する。ノード削除時は依存リンクも消す。
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
	// NodeEditorの右クリック判定から、保存用の文字列IDを取り出して後段のImGui Popupへ渡す。
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
	// ContextMenu取得はBegin/End中に行う。ライブラリが描画チャネルを使うためEnd後には呼ばない。
	NodeEditor::End();
	NodeEditor::SetCurrentEditor(nullptr);

	if (openNodeContextPopup) ImGui::OpenPopup("Gimmick Graph Node Context");
	if (openLinkContextPopup) ImGui::OpenPopup("Gimmick Graph Link Context");
	// 右クリック削除メニュー。ノード削除では接続リンクも同時に取り除く。
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

	// 編集途中の入力漏れ・不正リンクを保存前にも確認できるよう、常に現在状態を検証する。
	const auto validation = Gameplay::ValidateGimmickGraph(graph_);
	for (const auto& diagnostic : validation.diagnostics) {
		if (diagnostic.severity == Gameplay::DiagnosticSeverity::Error) ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", diagnostic.message.c_str());
	}
	// NodeEditorの位置変更も含めて前後JSONを比較し、変更分を既存のUndo/Redo履歴に登録する。
	const bool graphChanged = graphBeforeFrame.ToJson() != graph_.ToJson();
	if (graphChanged) {
		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) || valueInputActive) {
			// ノード移動中は各フレームを別Undoにせず、ドラッグ開始時の状態を一つだけ保持する。
			if (!pendingMouseEditStart_) pendingMouseEditStart_ = graphBeforeFrame;
		} else if (pendingMouseEditStart_) {
			AddGraphSnapshotCommand(document_, *pendingMouseEditStart_, graph_);
			pendingMouseEditStart_.reset();
		} else {
			AddGraphSnapshotCommand(document_, graphBeforeFrame, graph_);
		}
	} else if (pendingMouseEditStart_ && !ImGui::IsMouseDown(ImGuiMouseButton_Left) && !valueInputActive) {
		AddGraphSnapshotCommand(document_, *pendingMouseEditStart_, graph_);
		pendingMouseEditStart_.reset();
	}
	ImGui::End();
}

}

#else

namespace NoEngine::Editor {
void GimmickGraphPanel::Draw(ECS::Registry& registry, ECS::Entity selectedEntity) {
	static_cast<void>(registry);
	static_cast<void>(selectedEntity);
}
}

#endif // USE_IMGUI
