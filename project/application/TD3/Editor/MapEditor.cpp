#include "MapEditor.h"
#ifdef USE_IMGUI

#include "../Component/GridCellComponent.h"
#include "../MapData/MapLoader.h"
#include <algorithm>
#include <fstream>
#include <climits>
#include <cmath>

// ============================================================
//  Update
// ============================================================

void MapEditor::Update(No::Registry& registry) {
    dirty_ = false;

    ImGui::Begin("Map Editor", nullptr,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // ---- 左ペイン ----
    ImGui::BeginChild("##left", ImVec2(210.0f, 0.0f), true);
    DrawLeftPane();
    ImGui::EndChild();

    ImGui::SameLine();

    // ---- 中央ペイン（ビューポート） ----
    float vpW = static_cast<float>(gridWidth_) * CELL_SIZE;
    float vpH = static_cast<float>(gridHeight_) * CELL_SIZE;
    float childW = std::min(vpW + 4.0f, 520.0f);
    float childH = std::min(vpH + 4.0f, 520.0f);
    ImGui::BeginChild("##center", ImVec2(childW, childH), true,
        ImGuiWindowFlags_HorizontalScrollbar);
    DrawCenterPane();
    ImGui::EndChild();

    ImGui::SameLine();

    // ---- 右ペイン ----
    ImGui::BeginChild("##right", ImVec2(0.0f, 0.0f), true);
    DrawRightPane();
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
    currentTool_ = EditorTool::NodePlace;
    entityTypeToPlace_ = "player";

    saveStageNumber_ = 1;
    loadStageNumber_ = 1;
    showOverwritePopup_ = false;
    statusMessage_ = "";
    statusIsError_ = false;
    dirty_ = false;
}

// ============================================================
//  DrawLeftPane
// ============================================================

void MapEditor::DrawLeftPane() {

    // ========== グリッド設定 ==========
    ImGui::Text("Grid Settings");
    ImGui::Separator();

    ImGui::InputInt("Width", &pendingWidth_);
    ImGui::InputInt("Height", &pendingHeight_);
    ImGui::InputInt("Origin X", &pendingOriginX_);
    ImGui::InputInt("Origin Y", &pendingOriginY_);

    if (pendingWidth_ < 1) pendingWidth_ = 1;
    if (pendingHeight_ < 1) pendingHeight_ = 1;

    if (ImGui::Button("Apply Grid", ImVec2(-1, 0))) {
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
    ImGui::Text("Tools");
    ImGui::Separator();

    auto toolBtn = [&](const char* label, EditorTool tool) {
        bool active = (currentTool_ == tool);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }
        if (ImGui::Button(label, ImVec2(-1, 0))) {
            currentTool_ = tool;
            connectFirstNode_ = { INVALID, INVALID };
        }
        if (active) ImGui::PopStyleColor();
        };

    toolBtn("Node Place", EditorTool::NodePlace);
    toolBtn("Node Delete", EditorTool::NodeDelete);
    toolBtn("Connect", EditorTool::Connect);
    toolBtn("Disconnect", EditorTool::Disconnect);
    toolBtn("Entity Place", EditorTool::EntityPlace);
    toolBtn("Entity Delete", EditorTool::EntityDelete);

    if (currentTool_ == EditorTool::EntityPlace) {
        ImGui::Indent();
        ImGui::Text("Place as:");
        if (ImGui::RadioButton("Player", entityTypeToPlace_ == "player")) {
            entityTypeToPlace_ = "player";
        }
        if (ImGui::RadioButton("Enemy", entityTypeToPlace_ == "enemy")) {
            entityTypeToPlace_ = "enemy";
        }
        ImGui::Unindent();
    }

    ImGui::Spacing();

    // ========== ロード ==========
    ImGui::Text("Load");
    ImGui::Separator();
    ImGui::InputInt("##loadN", &loadStageNumber_);
    if (loadStageNumber_ < 1) loadStageNumber_ = 1;
    ImGui::SameLine();
    ImGui::Text("Stage");
    if (ImGui::Button("Load##btn", ImVec2(-1, 0))) {
        ExecuteLoad();
    }

    ImGui::Spacing();

    // ========== セーブ ==========
    ImGui::Text("Save");
    ImGui::Separator();
    ImGui::InputInt("##saveN", &saveStageNumber_);
    if (saveStageNumber_ < 1) saveStageNumber_ = 1;
    ImGui::SameLine();
    ImGui::Text("Stage");
    if (ImGui::Button("Save##btn", ImVec2(-1, 0))) {
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
//  DrawCenterPane
// ============================================================

void MapEditor::DrawCenterPane() {
    float vpW = static_cast<float>(gridWidth_) * CELL_SIZE;
    float vpH = static_cast<float>(gridHeight_) * CELL_SIZE;

    ImVec2 origin = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton("##vp", ImVec2(vpW, vpH));
    bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

    ImDrawList* dl = ImGui::GetWindowDrawList();

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
            fillColor = IM_COL32(0, 220, 220, 255);    // シアン: Connect 1回目
        } else if (coord == selectedNode_) {
            fillColor = IM_COL32(255, 220, 0, 255);    // 黄: 選択中
        } else {
            fillColor = IM_COL32(210, 210, 210, 255);  // 白: 通常
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

    // ---- クリック処理 ----
    if (clicked) {
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        int gx, gy;
        if (ScreenToGrid(mousePos, origin, gx, gy)) {
            HandleViewportClick(gx, gy);
        }
    }
}

// ============================================================
//  DrawRightPane
//  【修正】接続フラグの双方向同期を sync ラムダから直接代入に書き直した。
//  旧コードは bool& に三項演算子の結果を渡しており参照先が不定になっていた。
// ============================================================

void MapEditor::DrawRightPane() {
    ImGui::Text("Properties");
    ImGui::Separator();

    bool hasSelection = (selectedNode_.first != INVALID);
    if (!hasSelection) {
        ImGui::TextDisabled("No node selected");
        ImGui::TextDisabled("Click a node to select.");
        return;
    }

    auto it = nodes_.find(selectedNode_);
    if (it == nodes_.end()) {
        // 選択していたノードが削除された
        selectedNode_ = { INVALID, INVALID };
        ImGui::TextDisabled("No node selected");
        return;
    }

    MapData::NodeData& node = it->second;

    // ---- 座標表示 ----
    ImGui::Text("Node (%d, %d)", node.x, node.y);
    ImGui::Spacing();

    // ---- 接続フラグ ----
    ImGui::Text("Connections");

    bool changed = false;
    changed |= ImGui::Checkbox("Up", &node.up);
    changed |= ImGui::Checkbox("Right", &node.right);
    changed |= ImGui::Checkbox("Down", &node.down);
    changed |= ImGui::Checkbox("Left", &node.left);

    if (changed) {
        // ---- 双方向同期（直接代入）----
        // 自ノードの Up を変更したら、上隣ノードの Down も同じ値にする。
        // 隣ノードが存在しない場合は何もしない。
        auto syncNeighbor = [&](int nx, int ny, bool newVal,
            bool MapData::NodeData::* neighborField) {
                auto nit = nodes_.find({ nx, ny });
                if (nit != nodes_.end()) {
                    nit->second.*neighborField = newVal;
                }
            };

        syncNeighbor(node.x, node.y + 1, node.up, &MapData::NodeData::down);
        syncNeighbor(node.x + 1, node.y, node.right, &MapData::NodeData::left);
        syncNeighbor(node.x, node.y - 1, node.down, &MapData::NodeData::up);
        syncNeighbor(node.x - 1, node.y, node.left, &MapData::NodeData::right);

        dirty_ = true;
    }

    ImGui::Spacing();

    // ---- エンティティ設定 ----
    ImGui::Text("Entity");
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

    if (ImGui::RadioButton("None##prop", currentType == "none")) {
        if (entityIdx >= 0) entities_.erase(entities_.begin() + entityIdx);
    }
    if (ImGui::RadioButton("Player##prop", currentType == "player")) {
        // 既存の Player を全削除してから配置（1体制約）
        entities_.erase(
            std::remove_if(entities_.begin(), entities_.end(),
                [](const MapData::EntityData& e) { return e.type == "player"; }),
            entities_.end());
        MapData::EntityData ed;
        ed.type = "player"; ed.x = node.x; ed.y = node.y;
        entities_.push_back(ed);
    }
    if (ImGui::RadioButton("Enemy##prop", currentType == "enemy")) {
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
        ImGui::OpenPopup("Overwrite?");
        showOverwritePopup_ = false;
    }

    if (ImGui::BeginPopupModal("Overwrite?", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Stage %d はすでに存在します。", saveStageNumber_);
        ImGui::Text("上書きしますか？");
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(80.0f, 0.0f))) {
            ExecuteSave();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(80.0f, 0.0f))) {
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
    // X: 左→右（そのまま）
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
    outY = viewOriginY_ + gridHeight_ - 1 - cellY;  // Y 反転
    return true;
}

// ============================================================
//  HandleViewportClick
// ============================================================

void MapEditor::HandleViewportClick(int gx, int gy) {
    switch (currentTool_) {
    case EditorTool::NodePlace:
        PlaceNode(gx, gy);
        selectedNode_ = { gx, gy };
        break;

    case EditorTool::NodeDelete:
        DeleteNode(gx, gy);
        break;

    case EditorTool::Connect:
        HandleConnectClick(gx, gy, true);
        if (nodes_.count({ gx, gy })) selectedNode_ = { gx, gy };
        break;

    case EditorTool::Disconnect:
        HandleConnectClick(gx, gy, false);
        if (nodes_.count({ gx, gy })) selectedNode_ = { gx, gy };
        break;

    case EditorTool::EntityPlace:
        PlaceEntity(gx, gy);
        if (nodes_.count({ gx, gy })) selectedNode_ = { gx, gy };
        break;

    case EditorTool::EntityDelete:
        DeleteEntity(gx, gy);
        if (nodes_.count({ gx, gy })) selectedNode_ = { gx, gy };
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
    SetStatus("Node placed (" + std::to_string(gx) + ", " + std::to_string(gy) + ")", false);
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
    SetStatus("Node deleted (" + std::to_string(gx) + ", " + std::to_string(gy) + ")", false);
}

// ============================================================
//  HandleConnectClick
// ============================================================

void MapEditor::HandleConnectClick(int gx, int gy, bool isConnect) {
    if (!nodes_.count({ gx, gy })) {
        SetStatus("ノードがありません", true);
        connectFirstNode_ = { INVALID, INVALID };
        return;
    }

    if (connectFirstNode_.first == INVALID) {
        connectFirstNode_ = { gx, gy };
        SetStatus(std::string(isConnect ? "接続先" : "切断先") + "をクリックしてください", false);
    } else {
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

    SetStatus(entityTypeToPlace_ + " placed ("
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

    if (entities_.size() < before) SetStatus("Entity deleted", false);
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
        SetStatus("Stage " + std::to_string(saveStageNumber_) + " を保存しました", false);
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
        SetStatus("Stage " + std::to_string(loadStageNumber_) + " が見つかりません", true);
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
        dirty_ = true;

        SetStatus("Stage " + std::to_string(loadStageNumber_) + " を読み込みました", false);
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
            errorMsg = e.type + " が無効なノード上にいます ("
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