#include "MapEditor.h"
#ifdef USE_IMGUI

#include "../Component/GridCellComponent.h"
#include "../MapData/MapLoader.h"
#include <algorithm>
#include <fstream>
#include <climits>
#include <cmath>

void MapEditor::Update(No::Registry& registry) {
	dirty_ = false;

	ImGui::Begin("マップエディタ", nullptr,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	// ---- 左ペイン（ツール・設定） ----
	ImGui::BeginChild("##left", ImVec2(210.0f, 0.0f), true);
	DrawToolPanel();
	ImGui::EndChild();

	ImGui::SameLine();

	// ---- 中央ペイン（ビューポート） ----
	float vpW = static_cast<float>(gridWidth_) * CELL_SIZE;
	float vpH = static_cast<float>(gridHeight_) * CELL_SIZE;

	float padding = ImGui::GetStyle().ScrollbarSize; + 8.0f;                  // バー分 + 余白

	float childW = std::min(vpW + padding, 520.0f);
	float childH = std::min(vpH + padding, 520.0f);


	ImGui::BeginChild("##center", ImVec2(childW, childH), true,
		ImGuiWindowFlags_HorizontalScrollbar);
	DrawViewportWindow();
	ImGui::EndChild();

	ImGui::SameLine();

	// ---- 右ペイン（プロパティ） ----
	ImGui::BeginChild("##right", ImVec2(0.0f, 0.0f), true);
	DrawPropertiesPanel();
	ImGui::EndChild();

	// ---- 上書き確認ポップアップ（Begin/End の内側で呼ぶ） ----
	DrawOverwritePopup();

	ImGui::End();

	// ---- ECS 再構築（全 ImGui 処理の後） ----
	if (dirty_) {
		RebuildGridEntities(registry);
	}
}

// ============================================================
//  Reset
// ============================================================

void MapEditor::Reset() {
	nodes_.clear();
	entities_.clear();

	gridWidth_ = 10;
	gridHeight_ = 10;
	viewOriginX_ = 0;
	viewOriginY_ = 0;

	pendingWidth_ = 10;
	pendingHeight_ = 10;
	pendingOriginX_ = 0;
	pendingOriginY_ = 0;

	selectedNode_ = { INVALID, INVALID };
	connectFirstNode_ = { INVALID, INVALID };
	connectIsDisconnecting_ = false;
	currentTool_ = EditorTool::Node;
	entityTypeToPlace_ = "player";

	saveStageNumber_ = 1;
	loadStageNumber_ = 1;
	showOverwritePopup_ = false;
	statusMessage_ = "";
	statusIsError_ = false;
	dirty_ = false;
}

// ============================================================
//  DrawToolPanel  ("ツール・設定" ウィンドウ内容)
// ============================================================

void MapEditor::DrawToolPanel() {

	// ========== グリッド設定 ==========
	ImGui::Text("グリッド設定");
	ImGui::Separator();

	ImGui::InputInt("幅", &pendingWidth_);
	ImGui::InputInt("高さ", &pendingHeight_);
	ImGui::InputInt("原点 X", &pendingOriginX_);
	ImGui::InputInt("原点 Y", &pendingOriginY_);

	if (pendingWidth_ < 1) pendingWidth_ = 1;
	if (pendingHeight_ < 1) pendingHeight_ = 1;

	if (ImGui::Button("グリッドに適用", ImVec2(-1, 0))) {
		int newMaxX = pendingOriginX_ + pendingWidth_ - 1;
		int newMaxY = pendingOriginY_ + pendingHeight_ - 1;
		bool outOfRange = false;
		for (auto& [coord, node] : nodes_) {
			if (node.x < pendingOriginX_ || node.x > newMaxX ||
				node.y < pendingOriginY_ || node.y > newMaxY) {
				outOfRange = true;
				break;
			}
		}
		gridWidth_ = pendingWidth_;
		gridHeight_ = pendingHeight_;
		viewOriginX_ = pendingOriginX_;
		viewOriginY_ = pendingOriginY_;

		if (outOfRange) {
			SetStatus("警告: 範囲外のノードがあります", true);
		}
	}

	ImGui::Spacing();

	// ========== ツール選択 ==========
	ImGui::Text("ツール");
	ImGui::Separator();
	ImGui::TextDisabled("左クリック: 配置 / 右クリック: 削除");
	ImGui::Spacing();

	// アクティブなツールのボタンを強調表示するラムダ
	auto toolBtn = [&](const char* label, EditorTool tool) {
		bool active = (currentTool_ == tool);
		if (active) {
			ImGui::PushStyleColor(ImGuiCol_Button,
				ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		}
		if (ImGui::Button(label, ImVec2(-1, 0))) {
			currentTool_ = tool;
			connectFirstNode_ = { INVALID, INVALID };
			connectIsDisconnecting_ = false;
		}
		if (active) ImGui::PopStyleColor();
		};

	toolBtn("ノード", EditorTool::Node);
	toolBtn("接続 / 切断", EditorTool::Connect);
	toolBtn("エンティティ", EditorTool::Entity);

	// ---- エンティティ種別選択（Entity ツール選択中のみ表示）----
	if (currentTool_ == EditorTool::Entity) {
		ImGui::Indent();
		ImGui::Text("配置種別:");
		if (ImGui::RadioButton("プレイヤー", entityTypeToPlace_ == "player")) {
			entityTypeToPlace_ = "player";
		}
		if (ImGui::RadioButton("敵", entityTypeToPlace_ == "enemy")) {
			entityTypeToPlace_ = "enemy";
		}
		ImGui::Unindent();
	}

	// ---- 接続ツール操作状況表示（Connect ツール選択中のみ）----
	if (currentTool_ == EditorTool::Connect) {
		ImGui::Spacing();
		ImGui::TextDisabled("左クリック: 接続 (2点指定)");
		ImGui::TextDisabled("右クリック: 切断 (2点指定)");
		if (connectFirstNode_.first != INVALID) {
			const char* opLabel = connectIsDisconnecting_ ? "切断" : "接続";
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f),
				"%s: (%d, %d) → 次のノードを選択",
				opLabel,
				connectFirstNode_.first,
				connectFirstNode_.second);
		}
	}

	ImGui::Spacing();

	// ========== ロード ==========
	ImGui::Text("ロード");
	ImGui::Separator();
	ImGui::InputInt("##loadN", &loadStageNumber_);
	if (loadStageNumber_ < 1) loadStageNumber_ = 1;
	ImGui::SameLine();
	ImGui::Text("ステージ");
	if (ImGui::Button("ロード##btn", ImVec2(-1, 0))) {
		ExecuteLoad();
	}

	ImGui::Spacing();

	// ========== セーブ ==========
	ImGui::Text("セーブ");
	ImGui::Separator();
	ImGui::InputInt("##saveN", &saveStageNumber_);
	if (saveStageNumber_ < 1) saveStageNumber_ = 1;
	ImGui::SameLine();
	ImGui::Text("ステージ");
	if (ImGui::Button("セーブ##btn", ImVec2(-1, 0))) {
		std::string errMsg;
		if (!ValidateStage(errMsg)) {
			SetStatus(errMsg, true);
		} else if (FileExists(MapLoader::MakeManifestPath(saveStageNumber_))) {
			showOverwritePopup_ = true;
		} else {
			ExecuteSave();
		}
	}

	ImGui::Spacing();

	// ========== ステータスメッセージ ==========
	if (!statusMessage_.empty()) {
		ImGui::Separator();
		ImVec4 col = statusIsError_
			? ImVec4(1.0f, 0.3f, 0.3f, 1.0f)
			: ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
		ImGui::TextColored(col, "%s", statusMessage_.c_str());
	}
}

// ============================================================
//  DrawViewportWindow  ("マップビューポート" ウィンドウ内容)
//  左クリック・右クリック両方を検出する。
// ============================================================

void MapEditor::DrawViewportWindow() {
	float vpW = static_cast<float>(gridWidth_) * CELL_SIZE;
	float vpH = static_cast<float>(gridHeight_) * CELL_SIZE;

	ImVec2 origin = ImGui::GetCursorScreenPos();

	// クリック検出（左・右）
	ImGui::InvisibleButton("##vpBtn", ImVec2(vpW, vpH),
		ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	bool leftClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
	bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	// ---- 中央ガイドライン（グリッド線の前に描画してセルを薄く塗る） ----
	// 奇数: 中央1列/行,  偶数: 中央2列/行
	{
		ImU32 centerColor = IM_COL32(70, 70, 70, 50); // 半透明グレー

		// 中央列の範囲 [colStart, colEnd)
		int colStart = (gridWidth_ % 2 == 1)
			? gridWidth_ / 2
			: gridWidth_ / 2 - 1;
		int colEnd = gridWidth_ / 2 + 1;

		// 中央行の範囲 [rowStart, rowEnd)
		int rowStart = (gridHeight_ % 2 == 1)
			? gridHeight_ / 2
			: gridHeight_ / 2 - 1;
		int rowEnd = gridHeight_ / 2 + 1;

		// 縦帯（中央列）
		float rx0 = origin.x + colStart * CELL_SIZE;
		float rx1 = origin.x + colEnd * CELL_SIZE;
		dl->AddRectFilled(ImVec2(rx0, origin.y),
			ImVec2(rx1, origin.y + vpH), centerColor);

		// 横帯（中央行）
		float ry0 = origin.y + rowStart * CELL_SIZE;
		float ry1 = origin.y + rowEnd * CELL_SIZE;
		dl->AddRectFilled(ImVec2(origin.x, ry0),
			ImVec2(origin.x + vpW, ry1), centerColor);
	}
	// ---- グリッド枠線 ----
	ImU32 gridColor = IM_COL32(60, 60, 60, 255);
	for (int xi = 0; xi <= gridWidth_; xi++) {
		float sx = origin.x + xi * CELL_SIZE;
		dl->AddLine(ImVec2(sx, origin.y), ImVec2(sx, origin.y + vpH), gridColor);
	}
	for (int yi = 0; yi <= gridHeight_; yi++) {
		float sy = origin.y + yi * CELL_SIZE;
		dl->AddLine(ImVec2(origin.x, sy), ImVec2(origin.x + vpW, sy), gridColor);
	}

	// ---- 接続（エッジ）描画：right と up のみ（重複防止） ----
	ImU32 edgeColor = IM_COL32(200, 200, 200, 255);
	for (auto& [coord, node] : nodes_) {
		ImVec2 from = GridToScreen(node.x, node.y, origin);
		if (node.right && nodes_.count({ node.x + 1, node.y })) {
			dl->AddLine(from, GridToScreen(node.x + 1, node.y, origin), edgeColor, 2.0f);
		}
		if (node.up && nodes_.count({ node.x, node.y + 1 })) {
			dl->AddLine(from, GridToScreen(node.x, node.y + 1, origin), edgeColor, 2.0f);
		}
	}

	// ---- ノード描画 ----
	for (auto& [coord, node] : nodes_) {
		ImVec2 pos = GridToScreen(node.x, node.y, origin);

		ImU32 fillColor;
		if (coord == connectFirstNode_) {
			fillColor = IM_COL32(0, 220, 220, 255);   // シアン: 接続1回目選択中
		} else if (coord == selectedNode_) {
			fillColor = IM_COL32(255, 220, 0, 255);   // 黄: 選択中
		} else {
			fillColor = IM_COL32(210, 210, 210, 255); // 白: 通常
		}

		dl->AddCircleFilled(pos, 7.0f, fillColor);
		dl->AddCircle(pos, 7.0f, IM_COL32(0, 0, 0, 180), 16, 1.5f);
	}

	// ---- エンティティマーカー ----
	for (auto& entity : entities_) {
		ImVec2 pos = GridToScreen(entity.x, entity.y, origin);
		if (entity.type == "player") {
			dl->AddCircleFilled(pos, 11.0f, IM_COL32(30, 100, 255, 190));
			dl->AddText(ImVec2(pos.x - 4.0f, pos.y - 7.0f),
				IM_COL32(255, 255, 255, 255), "P");
		} else if (entity.type == "enemy") {
			dl->AddCircleFilled(pos, 11.0f, IM_COL32(255, 50, 50, 190));
			dl->AddText(ImVec2(pos.x - 4.0f, pos.y - 7.0f),
				IM_COL32(255, 255, 255, 255), "E");
		}
	}

	// ---- クリック処理（左・右） ----
	ImVec2 mousePos = ImGui::GetIO().MousePos;
	if (leftClicked) {
		int gx, gy;
		if (ScreenToGrid(mousePos, origin, gx, gy)) {
			HandleViewportClick(gx, gy, false);
		}
	}
	if (rightClicked) {
		int gx, gy;
		if (ScreenToGrid(mousePos, origin, gx, gy)) {
			HandleViewportClick(gx, gy, true);
		}
	}
}

// ============================================================
//  DrawPropertiesPanel  ("プロパティ" ウィンドウ内容)
// ============================================================

void MapEditor::DrawPropertiesPanel() {
	ImGui::Text("プロパティ");
	ImGui::Separator();

	bool hasSelection = (selectedNode_.first != INVALID);
	if (!hasSelection) {
		ImGui::TextDisabled("ノードが選択されていません");
		ImGui::TextDisabled("ノードをクリックして選択してください");
		return;
	}

	auto it = nodes_.find(selectedNode_);
	if (it == nodes_.end()) {
		// 選択していたノードが削除された
		selectedNode_ = { INVALID, INVALID };
		ImGui::TextDisabled("ノードが選択されていません");
		return;
	}

	MapData::NodeData& node = it->second;

	// ---- 座標表示 ----
	ImGui::Text("ノード (%d, %d)", node.x, node.y);
	ImGui::Spacing();

	// ---- 接続フラグ（十字配置） ----
	DrawCrossPanel(node);

	ImGui::Spacing();

	// ---- エンティティ設定 ----
	ImGui::Text("エンティティ");
	ImGui::Separator();

	std::string currentType = "none";
	int entityIdx = -1;
	for (int i = 0; i < static_cast<int>(entities_.size()); i++) {
		if (entities_[i].x == node.x && entities_[i].y == node.y) {
			currentType = entities_[i].type;
			entityIdx = i;
			break;
		}
	}

	if (ImGui::RadioButton("なし##prop", currentType == "none")) {
		if (entityIdx >= 0) entities_.erase(entities_.begin() + entityIdx);
	}
	if (ImGui::RadioButton("プレイヤー##prop", currentType == "player")) {
		// 既存の Player を全削除してから配置（1体制約）
		entities_.erase(
			std::remove_if(entities_.begin(), entities_.end(),
				[](const MapData::EntityData& e) { return e.type == "player"; }),
			entities_.end());
		MapData::EntityData ed;
		ed.type = "player"; ed.x = node.x; ed.y = node.y;
		entities_.push_back(ed);
	}
	if (ImGui::RadioButton("敵##prop", currentType == "enemy")) {
		if (entityIdx >= 0) entities_.erase(entities_.begin() + entityIdx);
		MapData::EntityData ed;
		ed.type = "enemy"; ed.x = node.x; ed.y = node.y;
		ed.enemyType = "normal";
		entities_.push_back(ed);
	}
}

// ============================================================
//  DrawOverwritePopup
// ============================================================

void MapEditor::DrawOverwritePopup() {
	if (showOverwritePopup_) {
		ImGui::OpenPopup("上書き確認");
		showOverwritePopup_ = false;
	}

	if (ImGui::BeginPopupModal("上書き確認", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("ステージ %d はすでに存在します。", saveStageNumber_);
		ImGui::Text("上書きしますか？");
		ImGui::Separator();

		if (ImGui::Button("はい", ImVec2(80.0f, 0.0f))) {
			ExecuteSave();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("いいえ", ImVec2(80.0f, 0.0f))) {
			SetStatus("保存をキャンセルしました", false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

// ============================================================
//  座標変換
// ============================================================

ImVec2 MapEditor::GridToScreen(int gx, int gy, ImVec2 origin) const {
	float sx = origin.x
		+ static_cast<float>(gx - viewOriginX_) * CELL_SIZE
		+ CELL_SIZE * 0.5f;

	// Y: ゲームは上向き正、ImGui は下向き正 → 反転
	float sy = origin.y
		+ static_cast<float>(viewOriginY_ + gridHeight_ - 1 - gy) * CELL_SIZE
		+ CELL_SIZE * 0.5f;

	return ImVec2(sx, sy);
}

bool MapEditor::ScreenToGrid(ImVec2 screenPos, ImVec2 origin, int& outX, int& outY) const {
	float relX = screenPos.x - origin.x;
	float relY = screenPos.y - origin.y;

	if (relX < 0.0f || relY < 0.0f) return false;

	int cellX = static_cast<int>(relX / CELL_SIZE);
	int cellY = static_cast<int>(relY / CELL_SIZE);

	if (cellX >= gridWidth_ || cellY >= gridHeight_) return false;

	outX = viewOriginX_ + cellX;
	outY = viewOriginY_ + gridHeight_ - 1 - cellY; // Y 反転
	return true;
}

// ============================================================
//  HandleViewportClick
//  isRightClick=false → 配置 / 接続
//  isRightClick=true  → 削除 / 切断
// ============================================================

void MapEditor::HandleViewportClick(int gx, int gy, bool isRightClick) {
	switch (currentTool_) {

		// ---- ノードツール ----
	case EditorTool::Node:
		if (!isRightClick) {
			PlaceNode(gx, gy);
			selectedNode_ = { gx, gy };
		} else {
			DeleteNode(gx, gy);
		}
		break;

		// ---- 接続ツール ----
	case EditorTool::Connect: {
		bool wantDisconnect = isRightClick;

		// 操作種別が変わった場合は 1 回目のクリックをリセットして新規操作を開始
		if (connectFirstNode_.first != INVALID &&
			connectIsDisconnecting_ != wantDisconnect) {
			connectFirstNode_ = { INVALID, INVALID };
		}

		connectIsDisconnecting_ = wantDisconnect;
		HandleConnectClick(gx, gy, !wantDisconnect);

		if (nodes_.count({ gx, gy })) selectedNode_ = { gx, gy };
		break;
	}

							// ---- エンティティツール ----
	case EditorTool::Entity:
		if (!isRightClick) {
			PlaceEntity(gx, gy);
			if (nodes_.count({ gx, gy })) selectedNode_ = { gx, gy };
		} else {
			DeleteEntity(gx, gy);
			if (nodes_.count({ gx, gy })) selectedNode_ = { gx, gy };
		}
		break;
	}
}

// ============================================================
//  PlaceNode
// ============================================================

void MapEditor::PlaceNode(int gx, int gy) {
	if (nodes_.count({ gx, gy })) return;

	MapData::NodeData node;
	node.x = gx; node.y = gy;
	node.up = node.right = node.down = node.left = false;
	nodes_[{ gx, gy }] = node;

	dirty_ = true;
	SetStatus("ノードを配置しました (" + std::to_string(gx) + ", " + std::to_string(gy) + ")", false);
}

// ============================================================
//  DeleteNode
// ============================================================

void MapEditor::DeleteNode(int gx, int gy) {
	auto it = nodes_.find({ gx, gy });
	if (it == nodes_.end()) return;

	// 隣接ノードの接続フラグを解除（双方向）
	if (nodes_.count({ gx,     gy + 1 })) nodes_.at({ gx,     gy + 1 }).down = false;
	if (nodes_.count({ gx + 1, gy })) nodes_.at({ gx + 1, gy }).left = false;
	if (nodes_.count({ gx,     gy - 1 })) nodes_.at({ gx,     gy - 1 }).up = false;
	if (nodes_.count({ gx - 1, gy })) nodes_.at({ gx - 1, gy }).right = false;

	nodes_.erase(it);

	// このノードのエンティティも削除
	entities_.erase(
		std::remove_if(entities_.begin(), entities_.end(),
			[gx, gy](const MapData::EntityData& e) {
				return e.x == gx && e.y == gy;
			}),
		entities_.end());

	if (selectedNode_ == std::make_pair(gx, gy)) selectedNode_ = { INVALID, INVALID };
	if (connectFirstNode_ == std::make_pair(gx, gy)) connectFirstNode_ = { INVALID, INVALID };

	dirty_ = true;
	SetStatus("ノードを削除しました (" + std::to_string(gx) + ", " + std::to_string(gy) + ")", false);
}

// ============================================================
//  HandleConnectClick
//  2 クリック方式。
//  isConnect=true→接続追加, false→接続削除
// ============================================================

void MapEditor::HandleConnectClick(int gx, int gy, bool isConnect) {
	if (!nodes_.count({ gx, gy })) {
		SetStatus("ノードがありません", true);
		connectFirstNode_ = { INVALID, INVALID };
		return;
	}

	if (connectFirstNode_.first == INVALID) {
		// 1 回目のクリック
		connectFirstNode_ = { gx, gy };
		SetStatus(
			std::string(isConnect ? "接続先" : "切断先") + "のノードをクリックしてください",
			false);
	} else {
		// 2 回目のクリック
		int ax = connectFirstNode_.first;
		int ay = connectFirstNode_.second;

		if (!AreAdjacent(ax, ay, gx, gy)) {
			SetStatus("隣接するノードのみ操作できます", true);
		} else if (isConnect) {
			AddConnection(ax, ay, gx, gy);
			SetStatus("接続しました", false);
		} else {
			RemoveConnection(ax, ay, gx, gy);
			SetStatus("切断しました", false);
		}

		connectFirstNode_ = { INVALID, INVALID };
	}
}

// ============================================================
//  PlaceEntity
// ============================================================

void MapEditor::PlaceEntity(int gx, int gy) {
	if (!nodes_.count({ gx, gy })) {
		SetStatus("ノード上にのみ配置できます", true);
		return;
	}

	// Player は 1 体のみ → 既存 Player を全削除
	if (entityTypeToPlace_ == "player") {
		entities_.erase(
			std::remove_if(entities_.begin(), entities_.end(),
				[](const MapData::EntityData& e) { return e.type == "player"; }),
			entities_.end());
	}

	// 同ノードの既存エンティティを上書き
	entities_.erase(
		std::remove_if(entities_.begin(), entities_.end(),
			[gx, gy](const MapData::EntityData& e) {
				return e.x == gx && e.y == gy;
			}),
		entities_.end());

	MapData::EntityData ed;
	ed.type = entityTypeToPlace_;
	ed.x = gx; ed.y = gy;
	if (entityTypeToPlace_ == "enemy") ed.enemyType = "normal";
	entities_.push_back(ed);

	std::string typeLabel = (entityTypeToPlace_ == "player") ? "プレイヤー" : "敵";
	SetStatus(typeLabel + "を配置しました ("
		+ std::to_string(gx) + ", " + std::to_string(gy) + ")", false);
}

// ============================================================
//  DeleteEntity
// ============================================================

void MapEditor::DeleteEntity(int gx, int gy) {
	size_t before = entities_.size();
	entities_.erase(
		std::remove_if(entities_.begin(), entities_.end(),
			[gx, gy](const MapData::EntityData& e) {
				return e.x == gx && e.y == gy;
			}),
		entities_.end());

	if (entities_.size() < before) {
		SetStatus("エンティティを削除しました", false);
	}
}

// ============================================================
//  AddConnection / RemoveConnection / AreAdjacent
// ============================================================

void MapEditor::AddConnection(int ax, int ay, int bx, int by) {
	if (!AreAdjacent(ax, ay, bx, by)) return;

	auto itA = nodes_.find({ ax, ay });
	auto itB = nodes_.find({ bx, by });
	if (itA == nodes_.end() || itB == nodes_.end()) return;

	int dx = bx - ax;
	int dy = by - ay;

	if (dx == 1) { itA->second.right = true;  itB->second.left = true; }
	if (dx == -1) { itA->second.left = true;  itB->second.right = true; }
	if (dy == 1) { itA->second.up = true;  itB->second.down = true; }
	if (dy == -1) { itA->second.down = true;  itB->second.up = true; }

	dirty_ = true;
}

void MapEditor::RemoveConnection(int ax, int ay, int bx, int by) {
	if (!AreAdjacent(ax, ay, bx, by)) return;

	auto itA = nodes_.find({ ax, ay });
	auto itB = nodes_.find({ bx, by });
	if (itA == nodes_.end() || itB == nodes_.end()) return;

	int dx = bx - ax;
	int dy = by - ay;

	if (dx == 1) { itA->second.right = false; itB->second.left = false; }
	if (dx == -1) { itA->second.left = false; itB->second.right = false; }
	if (dy == 1) { itA->second.up = false; itB->second.down = false; }
	if (dy == -1) { itA->second.down = false; itB->second.up = false; }

	dirty_ = true;
}

bool MapEditor::AreAdjacent(int ax, int ay, int bx, int by) const {
	int dx = std::abs(bx - ax);
	int dy = std::abs(by - ay);
	return (dx == 1 && dy == 0) || (dx == 0 && dy == 1);
}
void MapEditor::DrawCrossPanel(MapData::NodeData& node)
{
	ImGui::Text("接続方向");
	ImGui::Separator();

	const float size = 36.0f;
	ImVec2 btn(size, size);

	// トグルボタン（漢字表示）
	auto DirButton = [&](const char* label, bool& flag)
		{
			if (flag)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 180, 80, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(100, 200, 100, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(60, 160, 60, 255));
			}

			bool pressed = ImGui::Button(label, btn);

			if (flag)
				ImGui::PopStyleColor(3);

			if (pressed)
				flag = !flag;

			return pressed;
		};

	bool changed = false;

	// 中央揃え
	float fullWidth = size * 3.0f;
	float avail = ImGui::GetContentRegionAvail().x;
	float offset = (avail - fullWidth) * 0.5f;
	if (offset < 0) offset = 0;

	float baseX = ImGui::GetCursorPosX();

	// ---- 上 ----
	ImGui::SetCursorPosX(baseX + offset);
	ImGui::Dummy(btn);
	ImGui::SameLine();
	changed |= DirButton("上", node.up);
	ImGui::SameLine();
	ImGui::Dummy(btn);

	// ---- 左・中央・右 ----
	ImGui::SetCursorPosX(baseX + offset);
	changed |= DirButton("左", node.left);
	ImGui::SameLine();
	ImGui::Dummy(btn);
	ImGui::SameLine();
	changed |= DirButton("右", node.right);

	// ---- 下 ----
	ImGui::SetCursorPosX(baseX + offset);
	ImGui::Dummy(btn);
	ImGui::SameLine();
	changed |= DirButton("下", node.down);
	ImGui::SameLine();
	ImGui::Dummy(btn);

	if (changed)
	{
		auto syncNeighbor = [&](int nx, int ny, bool newVal,
			bool MapData::NodeData::* neighborField)
			{
				auto it = nodes_.find({ nx, ny });
				if (it != nodes_.end())
					it->second.*neighborField = newVal;
			};

		syncNeighbor(node.x, node.y + 1, node.up, &MapData::NodeData::down);
		syncNeighbor(node.x + 1, node.y, node.right, &MapData::NodeData::left);
		syncNeighbor(node.x, node.y - 1, node.down, &MapData::NodeData::up);
		syncNeighbor(node.x - 1, node.y, node.left, &MapData::NodeData::right);

		dirty_ = true;
	}
}
// ============================================================
//  ExecuteSave
// ============================================================

void MapEditor::ExecuteSave() {
	MapData::StageData stageData;
	stageData.connectionMap.gridScale = 1.0f;

	std::string stageStr = (saveStageNumber_ < 10 ? "0" : "") + std::to_string(saveStageNumber_);
	stageData.connectionMap.stageName = "stage_" + stageStr;

	for (auto& [coord, node] : nodes_) {
		stageData.connectionMap.nodes.push_back(node);
	}
	stageData.entityMap.entities = entities_;

	try {
		MapLoader::SaveStage(stageData, saveStageNumber_);
		SetStatus("ステージ " + std::to_string(saveStageNumber_) + " を保存しました", false);
	} catch (const std::exception& e) {
		SetStatus(std::string("保存エラー: ") + e.what(), true);
	}
}

// ============================================================
//  ExecuteLoad
// ============================================================

void MapEditor::ExecuteLoad() {
	std::string manifestPath = MapLoader::MakeManifestPath(loadStageNumber_);

	if (!FileExists(manifestPath)) {
		SetStatus("ステージ " + std::to_string(loadStageNumber_) + " が見つかりません", true);
		return;
	}

	try {
		MapData::StageData stageData = MapLoader::LoadStage(manifestPath);

		nodes_.clear();
		entities_.clear();

		for (auto& node : stageData.connectionMap.nodes) {
			nodes_[{ node.x, node.y }] = node;
		}
		entities_ = stageData.entityMap.entities;

		// ロードしたデータのバウンディングボックスにグリッドサイズを合わせる
		if (!nodes_.empty()) {
			int minX = INT_MAX, minY = INT_MAX;
			int maxX = INT_MIN, maxY = INT_MIN;
			for (auto& [coord, node] : nodes_) {
				if (node.x < minX) minX = node.x;
				if (node.y < minY) minY = node.y;
				if (node.x > maxX) maxX = node.x;
				if (node.y > maxY) maxY = node.y;
			}
			viewOriginX_ = minX - 1;
			viewOriginY_ = minY - 1;
			gridWidth_ = maxX - minX + 3;
			gridHeight_ = maxY - minY + 3;

			pendingOriginX_ = viewOriginX_;
			pendingOriginY_ = viewOriginY_;
			pendingWidth_ = gridWidth_;
			pendingHeight_ = gridHeight_;
		}

		selectedNode_ = { INVALID, INVALID };
		connectFirstNode_ = { INVALID, INVALID };
		connectIsDisconnecting_ = false;
		dirty_ = true;

		SetStatus("ステージ " + std::to_string(loadStageNumber_) + " を読み込みました", false);
	} catch (const std::exception& e) {
		SetStatus(std::string("読み込みエラー: ") + e.what(), true);
	}
}

// ============================================================
//  ValidateStage
// ============================================================

bool MapEditor::ValidateStage(std::string& errorMsg) const {
	if (nodes_.empty()) {
		errorMsg = "ノードが1つもありません";
		return false;
	}

	int playerCount = 0;
	for (auto& e : entities_) {
		if (e.type == "player") playerCount++;
	}
	if (playerCount == 0) {
		errorMsg = "プレイヤーが配置されていません";
		return false;
	}
	if (playerCount > 1) {
		errorMsg = "プレイヤーは1体のみです";
		return false;
	}

	for (auto& e : entities_) {
		if (!nodes_.count({ e.x, e.y })) {
			std::string typeLabel = (e.type == "player") ? "プレイヤー" : "敵";
			errorMsg = typeLabel + " が無効なノード上にいます ("
				+ std::to_string(e.x) + ", " + std::to_string(e.y) + ")";
			return false;
		}
	}

	for (auto& [coord, node] : nodes_) {
		int conn = (node.up ? 1 : 0) + (node.right ? 1 : 0)
			+ (node.down ? 1 : 0) + (node.left ? 1 : 0);
		if (conn == 0) {
			errorMsg = "孤立ノードがあります ("
				+ std::to_string(node.x) + ", " + std::to_string(node.y) + ")";
			return false;
		}
	}

	return true;
}

// ============================================================
//  FileExists
// ============================================================

bool MapEditor::FileExists(const std::string& path) const {
	std::ifstream f(path);
	return f.good();
}

// ============================================================
//  RebuildGridEntities
// ============================================================

void MapEditor::RebuildGridEntities(No::Registry& registry) {
	ClearGridEntities(registry);

	for (auto& [coord, node] : nodes_) {
		auto  entity = registry.GenerateEntity();
		auto* cell = registry.AddComponent<GridCellComponent>(entity);
		cell->gridX = node.x;
		cell->gridY = node.y;
		cell->hasConnectionUp = node.up;
		cell->hasConnectionRight = node.right;
		cell->hasConnectionDown = node.down;
		cell->hasConnectionLeft = node.left;
	}
}

// ============================================================
//  ClearGridEntities
// ============================================================

void MapEditor::ClearGridEntities(No::Registry& registry) {
	std::vector<No::Entity> toDestroy;
	auto view = registry.View<GridCellComponent>();
	for (auto e : view) toDestroy.push_back(e);
	for (auto e : toDestroy) registry.DestroyEntity(e);
}

// ============================================================
//  SetStatus
// ============================================================

void MapEditor::SetStatus(const std::string& msg, bool isError) {
	statusMessage_ = msg;
	statusIsError_ = isError;
}

#endif // USE_IMGUI