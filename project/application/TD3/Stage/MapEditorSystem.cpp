#include "application/TD3/Stage/MapEditorSystem.h"
#include "application/TD3/Stage/MapDataIO.h"
#include "application/TD3/Stage/MapLayerDef.h"
#include "application/TD3/Stage/GameStateComponent.h"
#include "application/TD3/Stage/MapRenderDirtyComponent.h"
#include "application/TD3/Stage/MapManager.h"
#include "application/TD3/Utility/GameResourceComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <cmath>
#include <cstring>

#ifdef USE_IMGUI

namespace {
	const Stage::TileLayerDef& GetLayerDefFromMode(int modeInt) {
		if (modeInt == 0) return Stage::GetFieldLayerDef();
		if (modeInt == 1) return Stage::GetBackgroundFrontLayerDef();
		if (modeInt == 2) return Stage::GetBackgroundBackLayerDef();
		return Stage::GetFieldLayerDef();
	}

	// フルリロード dirty flag を立てる
	void TriggerFullReload(No::Registry& registry, No::Entity managerE) {
		auto* dirty = registry.GetComponent<Stage::MapRenderDirtyComponent>(managerE);
		if (dirty) { dirty->fullReload = true; dirty->dirtyTiles.clear(); }
	}

	// レイヤーアルファの dirty flag を設定する
	void MarkAlphaDirty(No::Registry& registry, No::Entity managerE,
		Stage::TileLayer layer, float alpha) {
		auto* dirty = registry.GetComponent<Stage::MapRenderDirtyComponent>(managerE);
		if (!dirty) return;
		switch (layer) {
		case Stage::TileLayer::Field:
			dirty->fieldAlphaDirty = true; dirty->fieldAlpha = alpha; break;
		case Stage::TileLayer::BackgroundFront:
			dirty->bgFrontAlphaDirty = true; dirty->bgFrontAlpha = alpha; break;
		case Stage::TileLayer::BackgroundBack:
			dirty->bgBackAlphaDirty = true; dirty->bgBackAlpha = alpha; break;
		}
	}
}

void MapEditorSystem::Initialize(No::Entity managerEntity, No::Entity cam2dEntity,
	No::Entity playerEntity, Stage::MapManager* mapManager) {
	managerEntity_ = managerEntity;
	cam2dEntity_ = cam2dEntity;
	playerEntity_ = playerEntity;
	mapMgr_ = mapManager;
}

void MapEditorSystem::Update(No::Registry& registry, float /*dt*/) {
	// 初回: 空マップを生成してロード
	if (!initialized_) {
		initialized_ = true;
		//InitializeDefaultMap(registry);
	}

	// TAB で editorMode トグル（常に処理）
	HandleEditorToggle(registry);

	auto* state = registry.GetComponent<GameStateComponent>(managerEntity_);
	if (!state || !state->editorMode) return; // エディタモード以外は何もしない

	// --- エディタモード処理 ---
	DrawUnsavedDialog(registry);
	DrawEditorWindow(registry);
	HandleMouseInput(registry);
	HandleUndoRedo(registry);

	auto* cam2dT = registry.GetComponent<No::Transform2DComponent>(cam2dEntity_);
	if (cam2dT) {
		cam2dT->translate.x = gameCamX_;
		cam2dT->translate.y = gameCamY_;
	}
}


void MapEditorSystem::InitializeDefaultMap(No::Registry& registry) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;
	mapData->stageNo = 1;
	mapData->mapId = 'A';
	mapData->width = pendingWidth_;
	mapData->height = pendingHeight_;
	mapData->Initialize();
	pendingWidth_ = mapData->width;
	pendingHeight_ = mapData->height;
	ClampCameraToMap(*mapData);
	TriggerFullReload(registry, managerEntity_);
	ApplyLayerAlpha(registry);
}

void MapEditorSystem::HandleEditorToggle(No::Registry& registry) {
	auto* state = registry.GetComponent<GameStateComponent>(managerEntity_);
	if (!state || !No::Keyboard::IsTrigger(VK_TAB)) return;

	// 未保存の変更がある場合はダイアログを表示してトグルを保留
	if (state->editorMode && isDirty_) {
		pendingAction_ = PendingAction::SwitchToPlay;
		showUnsavedDialog_ = true;
		return;
	}

	DoEditorToggle(registry, state);
}

void MapEditorSystem::DoEditorToggle(No::Registry& registry, GameStateComponent* state) {
	state->editorMode = !state->editorMode;

	auto* dirty = registry.GetComponent<Stage::MapRenderDirtyComponent>(managerEntity_);

	if (!state->editorMode) {
		// エディタ → プレイ
		// isDirty_=false のときのみここに来る（true の場合はダイアログ経由で isDirty_ をクリア済み）
		if (mapMgr_) {
			auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
			if (mapData) mapMgr_->LoadMap(registry, managerEntity_, mapData->GetMapName());
		}
		if (dirty) {
			dirty->fieldAlphaDirty = true; dirty->fieldAlpha = 1.f;
			dirty->bgFrontAlphaDirty = true; dirty->bgFrontAlpha = 1.f;
			dirty->bgBackAlphaDirty = true; dirty->bgBackAlpha = 1.f;
		}
		auto* cam2dT = registry.GetComponent<No::Transform2DComponent>(cam2dEntity_);
		if (cam2dT) { cam2dT->translate.x = 0.f; cam2dT->translate.y = 0.f; }

	} else {
		// プレイ → エディタ
		DoLoad(registry);
		ApplyLayerAlpha(registry);

		auto* cam2dT = registry.GetComponent<No::Transform2DComponent>(cam2dEntity_);
		if (cam2dT) { cam2dT->translate.x = gameCamX_; cam2dT->translate.y = gameCamY_; }
	}
}

void MapEditorSystem::DrawUnsavedDialog(No::Registry& registry) {
	if (!showUnsavedDialog_) return;

	ImGui::OpenPopup("未保存の変更");
	if (ImGui::BeginPopupModal("未保存の変更", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("未保存の変更があります。保存しますか？");
		ImGui::Separator();

		if (ImGui::Button("保存して続行")) {
			DoSave(registry);
			ExecutePendingAction(registry);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("保存しない")) {
			isDirty_ = false;
			ExecutePendingAction(registry);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("キャンセル")) {
			pendingAction_ = PendingAction::None;
			showUnsavedDialog_ = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void MapEditorSystem::ExecutePendingAction(No::Registry& registry) {
	showUnsavedDialog_ = false;
	if (pendingAction_ == PendingAction::SwitchToPlay) {
		auto* state = registry.GetComponent<GameStateComponent>(managerEntity_);
		if (state) DoEditorToggle(registry, state);
	}
	pendingAction_ = PendingAction::None;
}

void MapEditorSystem::HandleMouseInput(No::Registry& registry) {
	if (!hasCanvasRect_) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	isCanvasDragging_ = isCanvasHovered_ && (
		ImGui::IsMouseDown(ImGuiMouseButton_Left) ||
		ImGui::IsMouseDown(ImGuiMouseButton_Right) ||
		ImGui::IsMouseDown(ImGuiMouseButton_Middle));

	// ---- 中ボタンドラッグ: パン ----
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && isCanvasHovered_) {
		isPanning_ = true;
		panStartX_ = io.MousePos.x;
		panStartY_ = io.MousePos.y;
		panCamStartX_ = camOffsetX_;
		panCamStartY_ = camOffsetY_;
	}
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
		isPanning_ = false;
	}
	if (isPanning_) {
		float dx = io.MousePos.x - panStartX_;
		float dy = io.MousePos.y - panStartY_;

		auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
		float mapW = mapData ? static_cast<float>(mapData->width) * tileDisplaySize_ : 0.f;
		float mapH = mapData ? static_cast<float>(mapData->height) * tileDisplaySize_ : 0.f;
		float maxX = std::max(0.f, mapW - canvasWidth_);
		float maxY = std::max(0.f, mapH - canvasHeight_);

		camOffsetX_ = std::clamp(panCamStartX_ - dx, 0.f, maxX);
		camOffsetY_ = std::clamp(panCamStartY_ - dy, 0.f, maxY);
	}

	if (!isCanvasHovered_) {
		return;
	}

	// ---- マウスホイール: スクロールパン ----
	if (!isPanning_ && std::fabs(io.MouseWheel) > 0.0f) {
		const float scrollStep = tileDisplaySize_;
		if (io.KeyShift) {
			camOffsetX_ -= io.MouseWheel * scrollStep;
		} else {
			camOffsetY_ -= io.MouseWheel * scrollStep;
		}

		//カメラオフセットをマップ範囲内に収める
		auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
		if (mapData) {
			ClampCameraToMap(*mapData);
		}
	}

	// ---- 左クリック: 配置、右クリック: 消去 ----
	bool leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
	bool rightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
	if (!leftDown && !rightDown) {
		return;
	}

	auto grid = ScreenToGrid(io.MousePos.x, io.MousePos.y);
	int gx = grid.first;
	int gy = grid.second;
	auto* mapDataBounds = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapDataBounds || !mapDataBounds->InBounds(gx, gy)) {
		return;
	}

	if (mode_ == EditorMode::Field) {
		if (leftDown) PlaceTile(registry, Stage::TileLayer::Field, gx, gy);
		if (rightDown) EraseTile(registry, Stage::TileLayer::Field, gx, gy);
	} else if (mode_ == EditorMode::BackgroundFront) {
		if (leftDown) PlaceTile(registry, Stage::TileLayer::BackgroundFront, gx, gy);
		if (rightDown) EraseTile(registry, Stage::TileLayer::BackgroundFront, gx, gy);
	} else if (mode_ == EditorMode::BackgroundBack) {
		if (leftDown) PlaceTile(registry, Stage::TileLayer::BackgroundBack, gx, gy);
		if (rightDown) EraseTile(registry, Stage::TileLayer::BackgroundBack, gx, gy);
	} else {
		if (leftDown) PlaceObject(registry, gx, gy);
		if (rightDown) EraseObject(registry, gx, gy);
	}
}

void MapEditorSystem::DrawEditorWindow(No::Registry& registry) {
	hasCanvasRect_ = false;
	isCanvasHovered_ = false;
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;
	if (isCanvasDragging_ || isPanning_) windowFlags |= ImGuiWindowFlags_NoMove;

	ImGui::Begin("MapTools");
	DrawSection_File(registry);
	DrawSection_MapSettings(registry);
	DrawSection_Layer(registry);
	DrawSection_Camera(registry);
	ImGui::End();

	if (!ImGui::Begin("MapEditor", nullptr, windowFlags)) { ImGui::End(); return; }

	if (ImGui::BeginTable("EditorLayout", 2,
		ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {

	
		// ---- 左ペイン: ツール（縦スクロール可） ----
	
	

		// ---- 右ペイン: マップキャンバス ----
		
		ImVec2 avail = ImGui::GetContentRegionAvail();
		if (ImGui::BeginChild("MapCanvas", avail, true,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {

			ImVec2 canvasPos = ImGui::GetCursorScreenPos();
			ImVec2 canvasSize = ImGui::GetContentRegionAvail();
			canvasPosX_ = canvasPos.x;
			canvasPosY_ = canvasPos.y;
			canvasWidth_ = canvasSize.x;
			canvasHeight_ = canvasSize.y;
			hasCanvasRect_ = (canvasWidth_ > 1.f && canvasHeight_ > 1.f);
			isCanvasHovered_ = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

			if (hasCanvasRect_) {
				// デフォルト 32x18 タイルが常にキャンバスに収まるスケールを計算（縮小のみ、拡大しない）
				constexpr float kBaseW = 32.f * Stage::TILE_SIZE;
				constexpr float kBaseH = 18.f * Stage::TILE_SIZE;
				float scale = std::min({ canvasWidth_ / kBaseW, canvasHeight_ / kBaseH, 1.f });
				tileDisplaySize_ = Stage::TILE_SIZE * scale;

				auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 clipMin(canvasPosX_, canvasPosY_);
				ImVec2 clipMax(canvasPosX_ + canvasWidth_, canvasPosY_ + canvasHeight_);
				dl->PushClipRect(clipMin, clipMax, true);

				const ImU32 outOfBoundsCol = IM_COL32(140, 140, 140, 255);
				const ImU32 mapAreaCol = IM_COL32(255, 255, 255, 255);
				dl->AddRectFilled(clipMin, clipMax, outOfBoundsCol);

				if (mapData) {
					No::Vector2 mapTL = GridToScreen(0, 0);
					No::Vector2 mapBR = GridToScreen(mapData->width, mapData->height);

					float visMinX = std::max(mapTL.x, clipMin.x);
					float visMinY = std::max(mapTL.y, clipMin.y);
					float visMaxX = std::min(mapBR.x, clipMax.x);
					float visMaxY = std::min(mapBR.y, clipMax.y);

					if (visMinX < visMaxX && visMinY < visMaxY) {
						dl->AddRectFilled(ImVec2(visMinX, visMinY), ImVec2(visMaxX, visMaxY), mapAreaCol);
					}
				}

				DrawMapTiles(registry, dl);
				if (mapData) {
					DrawObjectOverlay(dl, *mapData);
					DrawGridOverlay(dl, *mapData);
					ClampCameraToMap(*mapData);
				}

				dl->PopClipRect();
			}
			// キャンバス領域を ImGui に認識させる（ホバー/入力判定用）
			ImGui::Dummy(ImVec2(canvasWidth_, canvasHeight_));
		}
		ImGui::EndChild();

		ImGui::EndTable();
	}
	ImGui::End();
}

void MapEditorSystem::DrawSection_File(No::Registry& registry) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;

	ImGui::SeparatorText("ファイル");
	ImGui::InputInt("StageNo", &mapData->stageNo);
	mapData->stageNo = std::max(1, mapData->stageNo);

	char mapIdBuf[2] = { mapData->mapId, '\0' };
	if (ImGui::InputText("MapId", mapIdBuf, 2)) {
		if (mapIdBuf[0] >= 'A' && mapIdBuf[0] <= 'Z') {
			mapData->mapId = mapIdBuf[0];
		}
	}

	ImGui::Text("→  %s", mapData->GetMapName().c_str());

	if (ImGui::Button("Save")) {
		DoSave(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load")) {
		DoLoad(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		mapData->Initialize();
		isDirty_ = true;
		TriggerFullReload(registry, managerEntity_);
		ApplyLayerAlpha(registry);
	}
}

void MapEditorSystem::DrawSection_MapSettings(No::Registry& registry) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);

	ImGui::SeparatorText("マップ設定");
	ImGui::InputInt("幅", &pendingWidth_);
	ImGui::InputInt("高さ", &pendingHeight_);
	pendingWidth_ = std::clamp(pendingWidth_, 1, Stage::MAX_MAP_WIDTH);
	pendingHeight_ = std::clamp(pendingHeight_, 1, Stage::MAX_MAP_HEIGHT);
	ImGui::Text("現在: %d × %d",
		mapData ? mapData->width : 0,
		mapData ? mapData->height : 0);

	if (ImGui::Button("Resize（データ保持）") && mapData) {
		mapData->Resize(pendingWidth_, pendingHeight_);
		isDirty_ = true;
		ClampCameraToMap(*mapData);
		TriggerFullReload(registry, managerEntity_);
		ApplyLayerAlpha(registry);
	}
}

void MapEditorSystem::DrawSection_Layer(No::Registry& registry) {
	ImGui::SeparatorText("レイヤー");
	const char* modeNames[] = { "Field", "背景前", "背景後ろ", "Object" };
	int modeInt = static_cast<int>(mode_);
	if (ImGui::Combo("モード", &modeInt, modeNames, 4)) {
		mode_ = static_cast<EditorMode>(modeInt);
		selectedTexIdx_ = 1;
		selectedTileX_ = 0;
		selectedTileY_ = 0;
		ApplyLayerAlpha(registry);
	}

	if (mode_ == EditorMode::Object) {
		DrawObjectPalette(registry);
	} else {
		DrawTilePalette(registry);
	}
}

void MapEditorSystem::DrawSection_Camera(No::Registry& registry) {
	ImGui::SeparatorText("カメラ管理");

	ImGui::Text("エディタビュー: (%.0f, %.0f)", camOffsetX_, camOffsetY_);
	ImGui::SameLine();
	if (ImGui::Button("リセット##ev")) {
		camOffsetX_ = 0.f;
		camOffsetY_ = 0.f;
	}

	ImGui::Text("ゲームカメラ: (%.0f, %.0f)", gameCamX_, gameCamY_);
	ImGui::SameLine();
	if (ImGui::Button("リセット##gc")) {
		gameCamX_ = 0.f;
		gameCamY_ = 0.f;
	}

	ImGui::InputFloat("移動量 px", &gameCamStep_, 8.f, 40.f, "%.0f");
	gameCamStep_ = std::clamp(gameCamStep_, 1.f, 200.f);

	if (ImGui::Button("左")) gameCamX_ -= gameCamStep_;
	ImGui::SameLine();
	if (ImGui::Button("右")) gameCamX_ += gameCamStep_;
	ImGui::SameLine();
	if (ImGui::Button("上")) gameCamY_ -= gameCamStep_;
	ImGui::SameLine();
	if (ImGui::Button("下")) gameCamY_ += gameCamStep_;

	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (mapData) {
		float maxX = std::max(0.f, mapData->width * static_cast<float>(Stage::TILE_SIZE) - 1280.f);
		float maxY = std::max(0.f, mapData->height * static_cast<float>(Stage::TILE_SIZE) - 720.f);
		gameCamX_ = std::clamp(gameCamX_, 0.f, maxX);
		gameCamY_ = std::clamp(gameCamY_, 0.f, maxY);
	}
}

void MapEditorSystem::DrawTilePalette(No::Registry& registry) {
	const int modeInt = static_cast<int>(mode_);
	const Stage::TileLayerDef& def = GetLayerDefFromMode(modeInt);
	if (def.textureKeys.empty()) {
		ImGui::TextDisabled("利用可能なテクスチャがありません");
		return;
	}

	auto* res = registry.GetComponent<GameResourceComponent>(managerEntity_);
	if (!res) {
		return;
	}

	std::vector<const char*> texNames;
	for (const auto& k : def.textureKeys) {
		texNames.push_back(k.c_str());
	}

	int comboIdx = std::clamp(selectedTexIdx_ - 1, 0, static_cast<int>(texNames.size()) - 1);
	if (ImGui::Combo("使用画像", &comboIdx, texNames.data(), static_cast<int>(texNames.size()))) {
		selectedTexIdx_ = comboIdx + 1;
		selectedTileX_ = 0;
		selectedTileY_ = 0;
	}

	NoEngine::TextureRef tex = GetGameTextureOrWhite(*res, def.textureKeys[comboIdx]);
	ImTextureID texId = GetCachedImTextureID(def.textureKeys[comboIdx], tex);
	int texW = static_cast<int>(tex.GetWidth());
	int texH = static_cast<int>(tex.GetHeight());
	if (texW <= 0 || texH <= 0) {
		ImGui::TextDisabled("テクスチャサイズが取得できません");
		return;
	}

	int tilesX = std::max(1, texW / 64);
	int tilesY = std::max(1, texH / 64);
	float tileUvW = 1.f / static_cast<float>(tilesX);
	float tileUvH = 1.f / static_cast<float>(tilesY);

	float paletteH = std::min(static_cast<float>(tilesY * 48 + 8), 200.f);
	const float tileButtonSize = 44.f;
	const float tileOuterW = tileButtonSize + ImGui::GetStyle().FramePadding.x * 2.f;
	const float contentW = tilesX * tileOuterW + (tilesX - 1) * ImGui::GetStyle().ItemSpacing.x;
	ImGui::SetNextWindowContentSize(ImVec2(contentW, 0.f));
	ImGui::BeginChild("TilePalette", ImVec2(-1.f, paletteH), true, ImGuiWindowFlags_HorizontalScrollbar);

	for (int ty = 0; ty < tilesY; ty++) {
		for (int tx = 0; tx < tilesX; tx++) {
			bool selected = (selectedTexIdx_ == comboIdx + 1
				&& selectedTileX_ == tx
				&& selectedTileY_ == ty);

			ImVec2 uv0{ tx * tileUvW, ty * tileUvH };
			ImVec2 uv1{ uv0.x + tileUvW, uv0.y + tileUvH };
			ImVec4 tint = selected
				? ImVec4(1.f, 1.f, 1.f, 1.f)
				: ImVec4(0.4f, 0.4f, 0.4f, 1.f);

			ImGui::PushID(ty * tilesX + tx);
			if (ImGui::ImageButton("##tile", ImTextureRef(texId), ImVec2(tileButtonSize, tileButtonSize), uv0, uv1,
				ImVec4(0, 0, 0, 0), tint)) {
				selectedTexIdx_ = comboIdx + 1;
				selectedTileX_ = tx;
				selectedTileY_ = ty;
			}
			ImGui::PopID();

			if (tx < tilesX - 1) {
				ImGui::SameLine();
			}
		}
	}

	ImGui::EndChild();
}

void MapEditorSystem::DrawObjectPalette(No::Registry& registry) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;

	ImGui::SeparatorText("オブジェクト種別");
	const bool isMapA = (mapData->mapId == 'A');
	if (!isMapA && selectedObjType_ == 1) selectedObjType_ = 0; // 非Aマップで InitialSpawn 選択中なら戻す
	const char* objTypes[] = { "TransitionPoint", "InitialSpawn (Aマップのみ)", "RespawnPoint" };
	if (!isMapA) ImGui::BeginDisabled();
	ImGui::Combo("種別", &selectedObjType_, objTypes, 3);
	if (!isMapA) ImGui::EndDisabled();

	float listHeight = std::clamp(ImGui::GetContentRegionAvail().y - 60.f, 80.f, 400.f);
	ImGui::BeginChild("##obj_list", ImVec2(0, listHeight), true);

	// --- TransitionPoint ---
	ImGui::SeparatorText("配置済み TransitionPoint");
	for (int i = 0; i < static_cast<int>(mapData->transitions.size()); i++) {
		auto& tp = mapData->transitions[i];
		ImGui::PushID(i);

		char idBuf[16];
		::strncpy_s(idBuf, sizeof(idBuf), tp.id.c_str(), _TRUNCATE);
		ImGui::SetNextItemWidth(40.f);
		if (ImGui::InputText("id", idBuf, sizeof(idBuf))) {
			tp.id = idBuf;
			isDirty_ = true;
		}

		ImGui::SameLine();
		ImGui::Text("(%d,%d)", tp.x, tp.y);

		char tmBuf[32];
		::strncpy_s(tmBuf, sizeof(tmBuf), tp.targetMap.c_str(), _TRUNCATE);
		ImGui::SetNextItemWidth(100.f);
		if (ImGui::InputText("->Map", tmBuf, sizeof(tmBuf))) {
			tp.targetMap = tmBuf;
			isDirty_ = true;
		}

		ImGui::SameLine();
		char tpBuf[16];
		::strncpy_s(tpBuf, sizeof(tpBuf), tp.targetPoint.c_str(), _TRUNCATE);
		ImGui::SetNextItemWidth(40.f);
		if (ImGui::InputText("->Pt", tpBuf, sizeof(tpBuf))) {
			tp.targetPoint = tpBuf;
			isDirty_ = true;
		}

		if (ImGui::Button("削除")) {
			PushObjectUndo(*mapData);
			mapData->transitions.erase(mapData->transitions.begin() + i);
			isDirty_ = true;
			ImGui::PopID();
			break;
		}
		ImGui::Separator();
		ImGui::PopID();
	}

	// --- InitialSpawn ---
	ImGui::SeparatorText("InitialSpawn");
	ImGui::Text("(%d, %d)", mapData->initialSpawn.x, mapData->initialSpawn.y);

	// --- RespawnPoint ---
	ImGui::SeparatorText("配置済み RespawnPoint");
	for (int i = 0; i < static_cast<int>(mapData->respawnPoints.size()); i++) {
		auto& rp = mapData->respawnPoints[i];
		ImGui::PushID(1000 + i);

		char idBuf[16];
		::strncpy_s(idBuf, sizeof(idBuf), rp.id.c_str(), _TRUNCATE);
		ImGui::SetNextItemWidth(40.f);
		if (ImGui::InputText("id", idBuf, sizeof(idBuf))) {
			rp.id = idBuf;
			isDirty_ = true;
		}

		ImGui::SameLine();
		ImGui::Text("(%d,%d)", rp.x, rp.y);

		if (ImGui::Button("削除")) {
			PushObjectUndo(*mapData);
			mapData->respawnPoints.erase(mapData->respawnPoints.begin() + i);
			isDirty_ = true;
			ImGui::PopID();
			break;
		}
		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::Text("LClick: 配置   RClick: 削除");
}

void MapEditorSystem::DrawMapTiles(No::Registry& registry, ImDrawList* dl) {
	auto* res = registry.GetComponent<GameResourceComponent>(managerEntity_);
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!res || !mapData) return;

	float ts = tileDisplaySize_;
	int colStart = static_cast<int>(camOffsetX_ / ts);
	int rowStart = static_cast<int>(camOffsetY_ / ts);
	int colEnd = std::min(static_cast<int>((camOffsetX_ + canvasWidth_) / ts) + 1, mapData->width);
	int rowEnd = std::min(static_cast<int>((camOffsetY_ + canvasHeight_) / ts) + 1, mapData->height);
	colStart = std::max(colStart, 0);
	rowStart = std::max(rowStart, 0);

	auto drawLayer = [&](const std::vector<std::vector<Stage::TileCell>>& tiles,
		const Stage::TileLayerDef& layerDef, float alpha) {
			ImU32 tint = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, alpha));
			for (int gy = rowStart; gy < rowEnd; gy++) {
				for (int gx = colStart; gx < colEnd; gx++) {
					const Stage::TileCell& cell = tiles[gy][gx];
					if (cell.IsEmpty()) continue;

					int idx = cell.textureIdx - 1;
					if (idx < 0 || idx >= static_cast<int>(layerDef.textureKeys.size())) continue;

					NoEngine::TextureRef tex = GetGameTextureOrWhite(*res, layerDef.textureKeys[idx]);
					float texW = static_cast<float>(tex.GetWidth());
					float texH = static_cast<float>(tex.GetHeight());
					if (texW <= 0.f || texH <= 0.f) continue;

					ImTextureID texId = GetCachedImTextureID(layerDef.textureKeys[idx], tex);
					DrawTile(dl, texId, gx, gy, tint, cell, texW, texH);
				}
			}
		};

	// 奥から前: BackgroundBack → BackgroundFront → Field の順で描画
	drawLayer(mapData->bgTilesBack, Stage::GetBackgroundBackLayerDef(), mode_ == EditorMode::BackgroundBack ? 1.f : 0.4f);
	drawLayer(mapData->bgTilesFront, Stage::GetBackgroundFrontLayerDef(), mode_ == EditorMode::BackgroundFront ? 1.f : 0.4f);
	drawLayer(mapData->fieldTiles, Stage::GetFieldLayerDef(), mode_ == EditorMode::Field ? 1.f : 0.4f);
}

void MapEditorSystem::DrawTile(ImDrawList* dl, ImTextureID tex,
	int gx, int gy, ImU32 tint,
	const Stage::TileCell& cell, float texW, float texH) const {
	float tileUvW = 64.f / texW;
	float tileUvH = 64.f / texH;
	ImVec2 uv0{ cell.tileX * tileUvW, cell.tileY * tileUvH };
	ImVec2 uv1{ uv0.x + tileUvW, uv0.y + tileUvH };

	auto scrTL = GridToScreen(gx, gy);
	auto scrBR = GridToScreen(gx + 1, gy + 1);
	dl->AddImage(ImTextureRef(tex),
		ImVec2(scrTL.x, scrTL.y), ImVec2(scrBR.x, scrBR.y),
		uv0, uv1, tint);
}

void MapEditorSystem::DrawGridOverlay(ImDrawList* dl, const Stage::MapData& mapData) const {
	const ImU32 gridCol = IM_COL32(80, 80, 80, 120);
	const ImU32 borderCol = IM_COL32(200, 200, 200, 200);

	float ts = tileDisplaySize_;
	No::Vector2 mapTL = GridToScreen(0, 0);
	No::Vector2 mapBR = GridToScreen(mapData.width, mapData.height);

	float visLeft = std::max(mapTL.x, canvasPosX_);
	float visTop = std::max(mapTL.y, canvasPosY_);
	float visRight = std::min(mapBR.x, canvasPosX_ + canvasWidth_);
	float visBottom = std::min(mapBR.y, canvasPosY_ + canvasHeight_);

	if (visLeft >= visRight || visTop >= visBottom) {
		dl->AddRect(ImVec2(mapTL.x, mapTL.y), ImVec2(mapBR.x, mapBR.y), borderCol, 0.f, 0, 2.f);
		return;
	}

	int colStart = static_cast<int>(camOffsetX_ / ts);
	int colEnd = std::min(static_cast<int>((camOffsetX_ + canvasWidth_) / ts) + 1, mapData.width);
	for (int cx = colStart; cx <= colEnd; cx++) {
		float sx = canvasPosX_ + cx * ts - camOffsetX_;
		if (sx < visLeft || sx > visRight) continue;
		dl->AddLine(ImVec2(sx, visTop), ImVec2(sx, visBottom), gridCol, 1.f);
	}

	int rowStart = static_cast<int>(camOffsetY_ / ts);
	int rowEnd = std::min(static_cast<int>((camOffsetY_ + canvasHeight_) / ts) + 1, mapData.height);
	for (int ry = rowStart; ry <= rowEnd; ry++) {
		float sy = canvasPosY_ + ry * ts - camOffsetY_;
		if (sy < visTop || sy > visBottom) continue;
		dl->AddLine(ImVec2(visLeft, sy), ImVec2(visRight, sy), gridCol, 1.f);
	}

	dl->AddRect(ImVec2(mapTL.x, mapTL.y), ImVec2(mapBR.x, mapBR.y), borderCol, 0.f, 0, 2.f);
}

void MapEditorSystem::DrawObjectOverlay(ImDrawList* dl, const Stage::MapData& mapData) const {
	const ImU32 tpCol = IM_COL32(255, 200, 0, 200); // 黄: TransitionPoint
	const ImU32 ispCol = IM_COL32(50, 220, 50, 200); // 緑: InitialSpawn
	const ImU32 rspCol = IM_COL32(50, 200, 255, 200); // 水色: RespawnPoint
	const ImU32 txtCol = IM_COL32(255, 255, 255, 255);

	// TransitionPoint
	for (const auto& tp : mapData.transitions) {
		No::Vector2 scrTL = GridToScreen(tp.x, tp.y);
		No::Vector2 scrBR = GridToScreen(tp.x + 1, tp.y + 1);
		dl->AddRect(ImVec2(scrTL.x, scrTL.y), ImVec2(scrBR.x, scrBR.y), tpCol, 0.f, 0, 2.f);
		dl->AddText(ImVec2(scrTL.x + 2.f, scrTL.y + 2.f), txtCol, tp.id.c_str());
	}

	// InitialSpawn (StageXX_A のみ表示)
	if (mapData.mapId == 'A') {
		const auto& isp = mapData.initialSpawn;
		No::Vector2 scrTL = GridToScreen(isp.x, isp.y);
		No::Vector2 scrBR = GridToScreen(isp.x + 1, isp.y + 1);
		dl->AddRectFilled(ImVec2(scrTL.x, scrTL.y), ImVec2(scrBR.x, scrBR.y), IM_COL32(50, 220, 50, 60));
		dl->AddRect(ImVec2(scrTL.x, scrTL.y), ImVec2(scrBR.x, scrBR.y), ispCol, 0.f, 0, 2.f);
		dl->AddText(ImVec2(scrTL.x + 2.f, scrTL.y + 2.f), txtCol, "IS");
	}

	// RespawnPoint
	for (const auto& rp : mapData.respawnPoints) {
		No::Vector2 scrTL = GridToScreen(rp.x, rp.y);
		No::Vector2 scrBR = GridToScreen(rp.x + 1, rp.y + 1);
		dl->AddRectFilled(ImVec2(scrTL.x, scrTL.y), ImVec2(scrBR.x, scrBR.y), IM_COL32(50, 200, 255, 60));
		dl->AddRect(ImVec2(scrTL.x, scrTL.y), ImVec2(scrBR.x, scrBR.y), rspCol, 0.f, 0, 2.f);
		dl->AddText(ImVec2(scrTL.x + 2.f, scrTL.y + 2.f), txtCol, rp.id.c_str());
	}
}

void MapEditorSystem::ApplyLayerAlpha(No::Registry& registry) {
	switch (mode_) {
	case EditorMode::Field:
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::Field, 1.f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundFront, 0.4f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundBack, 0.4f);
		break;
	case EditorMode::BackgroundFront:
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::Field, 0.4f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundFront, 1.f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundBack, 0.4f);
		break;
	case EditorMode::BackgroundBack:
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::Field, 0.4f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundFront, 0.4f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundBack, 1.f);
		break;
	case EditorMode::Object:
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::Field, 0.4f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundFront, 0.4f);
		MarkAlphaDirty(registry, managerEntity_, Stage::TileLayer::BackgroundBack, 0.4f);
		break;
	}
}

ImTextureID MapEditorSystem::GetCachedImTextureID(const std::string& key, const NoEngine::TextureRef& tex) {
	auto it = texIdCache_.find(key);
	if (it != texIdCache_.end()) {
		return it->second;
	}

	NoEngine::DescriptorHandle slot = NoEngine::Render::gTextureHeap.Alloc();
	NoEngine::GraphicsCore::sGraphicsDevice->GetDevice()->CopyDescriptorsSimple(
		1,
		static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(slot),
		tex.GetSRV(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ImTextureID id = static_cast<ImTextureID>(slot.GetGpuPtr());
	texIdCache_[key] = id;
	return id;
}

void MapEditorSystem::PlaceTile(No::Registry& registry, Stage::TileLayer layer, int x, int y) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData || !mapData->InBounds(x, y)) return;

	auto& tiles = (layer == Stage::TileLayer::Field) ? mapData->fieldTiles
		: (layer == Stage::TileLayer::BackgroundFront) ? mapData->bgTilesFront
		: mapData->bgTilesBack;
	Stage::TileCell newCell{ selectedTexIdx_, selectedTileX_, selectedTileY_ };
	if (tiles[y][x] == newCell) return;

	PushTileUndo({ layer, x, y, tiles[y][x], newCell });
	tiles[y][x] = newCell;

	auto* dirty = registry.GetComponent<Stage::MapRenderDirtyComponent>(managerEntity_);
	if (dirty) dirty->dirtyTiles.push_back({ layer, x, y });
	isDirty_ = true;
}

void MapEditorSystem::EraseTile(No::Registry& registry, Stage::TileLayer layer, int x, int y) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData || !mapData->InBounds(x, y)) return;

	auto& tiles = (layer == Stage::TileLayer::Field) ? mapData->fieldTiles
		: (layer == Stage::TileLayer::BackgroundFront) ? mapData->bgTilesFront
		: mapData->bgTilesBack;
	if (tiles[y][x].IsEmpty()) return;

	PushTileUndo({ layer, x, y, tiles[y][x], Stage::TileCell{} });
	tiles[y][x] = Stage::TileCell{};

	auto* dirty = registry.GetComponent<Stage::MapRenderDirtyComponent>(managerEntity_);
	if (dirty) dirty->dirtyTiles.push_back({ layer, x, y });
	isDirty_ = true;
}

void MapEditorSystem::PlaceObject(No::Registry& registry, int x, int y) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;

	if (selectedObjType_ == 0) { // TransitionPoint
		for (const auto& tp : mapData->transitions) {
			if (tp.x == x && tp.y == y) return;
		}
		PushObjectUndo(*mapData);
		Stage::TransitionPoint tp;
		tp.id = std::to_string(mapData->transitions.size() + 1);
		tp.x = x; tp.y = y;
		mapData->transitions.push_back(tp);
	} else if (selectedObjType_ == 1) { // InitialSpawn (StageXX_A のみ・上書き)
		if (mapData->mapId != 'A') return;
		PushObjectUndo(*mapData);
		mapData->initialSpawn = { x, y };
	} else if (selectedObjType_ == 2) { // RespawnPoint
		for (const auto& rp : mapData->respawnPoints) {
			if (rp.x == x && rp.y == y) return;
		}
		PushObjectUndo(*mapData);
		Stage::RespawnPoint rp;
		rp.id = std::to_string(mapData->respawnPoints.size() + 1);
		rp.x = x; rp.y = y;
		mapData->respawnPoints.push_back(rp);
	}
	isDirty_ = true;
}

void MapEditorSystem::EraseObject(No::Registry& registry, int x, int y) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;

	if (selectedObjType_ == 0) { // TransitionPoint
		auto& ts = mapData->transitions;
		auto it = std::find_if(ts.begin(), ts.end(),
			[&](const Stage::TransitionPoint& tp) { return tp.x == x && tp.y == y; });
		if (it == ts.end()) return;
		PushObjectUndo(*mapData);
		ts.erase(it);
	} else if (selectedObjType_ == 1) { // InitialSpawn は右クリック削除なし
		return;
	} else if (selectedObjType_ == 2) { // RespawnPoint
		auto& rps = mapData->respawnPoints;
		auto it = std::find_if(rps.begin(), rps.end(),
			[&](const Stage::RespawnPoint& rp) { return rp.x == x && rp.y == y; });
		if (it == rps.end()) return;
		PushObjectUndo(*mapData);
		rps.erase(it);
	}
	isDirty_ = true;
}

void MapEditorSystem::PushTileUndo(const TileEditCommand& cmd) {
	tileRedoStack_.clear();
	objRedoStack_.clear();
	tileUndoStack_.push_back(cmd);
	if (static_cast<int>(tileUndoStack_.size()) > kMaxUndoHistory)
		tileUndoStack_.erase(tileUndoStack_.begin());
}

void MapEditorSystem::PushObjectUndo(const Stage::MapData& before) {
	tileRedoStack_.clear();
	objRedoStack_.clear();
	objUndoStack_.push_back({ before.initialSpawn, before.respawnPoints, before.transitions });
	if (static_cast<int>(objUndoStack_.size()) > kMaxUndoHistory)
		objUndoStack_.erase(objUndoStack_.begin());
}

void MapEditorSystem::HandleUndoRedo(No::Registry& registry) {
	auto& io = ImGui::GetIO();
	if (!io.KeyCtrl) return;

	const bool doUndo = ImGui::IsKeyPressed(ImGuiKey_Z);
	const bool doRedo = ImGui::IsKeyPressed(ImGuiKey_Y);
	if (!doUndo && !doRedo) return;

	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	auto* dirty = registry.GetComponent<Stage::MapRenderDirtyComponent>(managerEntity_);
	if (!mapData || !dirty) return;

	if (doUndo) {
		if (!tileUndoStack_.empty()) {
			auto cmd = tileUndoStack_.back(); tileUndoStack_.pop_back();
			auto& tiles = (cmd.layer == Stage::TileLayer::Field) ? mapData->fieldTiles
				: (cmd.layer == Stage::TileLayer::BackgroundFront) ? mapData->bgTilesFront
				: mapData->bgTilesBack;
			tiles[cmd.y][cmd.x] = cmd.before;
			dirty->dirtyTiles.push_back({ cmd.layer, cmd.x, cmd.y });
			tileRedoStack_.push_back(cmd);
			isDirty_ = true;
		} else if (!objUndoStack_.empty()) {
			objRedoStack_.push_back({ mapData->initialSpawn, mapData->respawnPoints, mapData->transitions });
			auto& snap = objUndoStack_.back();
			mapData->initialSpawn = snap.initialSpawn;
			mapData->respawnPoints = snap.respawnPoints;
			mapData->transitions = snap.transitions;
			objUndoStack_.pop_back();
			isDirty_ = true;
		}
	}

	if (doRedo) {
		if (!tileRedoStack_.empty()) {
			auto cmd = tileRedoStack_.back(); tileRedoStack_.pop_back();
			auto& tiles = (cmd.layer == Stage::TileLayer::Field) ? mapData->fieldTiles
				: (cmd.layer == Stage::TileLayer::BackgroundFront) ? mapData->bgTilesFront
				: mapData->bgTilesBack;
			tiles[cmd.y][cmd.x] = cmd.after;
			dirty->dirtyTiles.push_back({ cmd.layer, cmd.x, cmd.y });
			tileUndoStack_.push_back(cmd);
			isDirty_ = true;
		} else if (!objRedoStack_.empty()) {
			objUndoStack_.push_back({ mapData->initialSpawn, mapData->respawnPoints, mapData->transitions });
			auto& snap = objRedoStack_.back();
			mapData->initialSpawn = snap.initialSpawn;
			mapData->respawnPoints = snap.respawnPoints;
			mapData->transitions = snap.transitions;
			objRedoStack_.pop_back();
			isDirty_ = true;
		}
	}
}

void MapEditorSystem::DoSave(No::Registry& registry) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;
	Stage::MapDataIO::Save(*mapData);
	isDirty_ = false;
}

void MapEditorSystem::DoLoad(No::Registry& registry) {
	auto* mapData = registry.GetComponent<Stage::MapData>(managerEntity_);
	if (!mapData) return;
	Stage::MapData loaded;
	if (Stage::MapDataIO::Load(loaded, mapData->GetMapName())) {
		*mapData = loaded;
		pendingWidth_ = mapData->width;
		pendingHeight_ = mapData->height;
		ClampCameraToMap(*mapData);
		TriggerFullReload(registry, managerEntity_);
		ApplyLayerAlpha(registry);
		isDirty_ = false;
	}
}

std::pair<int, int> MapEditorSystem::ScreenToGrid(float screenX, float screenY) const {
	float worldX = (screenX - canvasPosX_) + camOffsetX_;
	float worldY = (screenY - canvasPosY_) + camOffsetY_;
	int gx = static_cast<int>(std::floor(worldX / tileDisplaySize_));
	int gy = static_cast<int>(std::floor(worldY / tileDisplaySize_));
	return { gx, gy };
}

No::Vector2 MapEditorSystem::GridToScreen(int gx, int gy) const {
	float sx = canvasPosX_ + gx * tileDisplaySize_ - camOffsetX_;
	float sy = canvasPosY_ + gy * tileDisplaySize_ - camOffsetY_;
	return { sx, sy };
}

void MapEditorSystem::ClampCameraToMap(const Stage::MapData& mapData) {

	float mapW = static_cast<float>(mapData.width) * tileDisplaySize_;
	float mapH = static_cast<float>(mapData.height) * tileDisplaySize_;
	float viewW = std::max(1.f, canvasWidth_);
	float viewH = std::max(1.f, canvasHeight_);
	float maxX = std::max(0.f, mapW - viewW);
	float maxY = std::max(0.f, mapH - viewH);
	camOffsetX_ = std::clamp(camOffsetX_, 0.f, maxX);
	camOffsetY_ = std::clamp(camOffsetY_, 0.f, maxY);
}

#endif // USE_IMGUI

