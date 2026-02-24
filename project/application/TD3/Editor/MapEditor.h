#pragma once

#ifdef USE_IMGUI

#include "engine/NoEngine.h"
#include "../MapData/StageData.h"
#include "externals/imgui/imgui.h"
#include <map>
#include <vector>
#include <string>
#include <climits>

/// <summary>
/// エディタツールモード
/// </summary>
enum class EditorTool {
	NodePlace,
	NodeDelete,
	Connect,
	Disconnect,
	EntityPlace,
	EntityDelete
};

/// <summary>
/// マップエディタ
/// </summary>
class MapEditor {
public:
	void Update(No::Registry& registry);
	void RebuildGridEntities(No::Registry& registry);
	void Reset();

private:
	// ============================================================
	//  内部データ
	// ============================================================

	/// ノード一覧。キーはグリッド座標 (x, y)。
	std::map<std::pair<int, int>, MapData::NodeData> nodes_;

	/// エンティティ一覧（"player" / "enemy"）。ECS には登録しない。
	std::vector<MapData::EntityData> entities_;

	// ============================================================
	//  グリッド設定
	// ============================================================

	int gridWidth_ = 10;  // 編集可能範囲（幅）
	int gridHeight_ = 10;  // 編集可能範囲（高さ）
	int viewOriginX_ = 0;   // ビューポート左端のグリッド X 座標
	int viewOriginY_ = 0;   // ビューポート下端のグリッド Y 座標（ゲーム座標）

	// Apply ボタンを押すまで実際の値に反映しない pending 値
	int pendingWidth_ = 10;
	int pendingHeight_ = 10;
	int pendingOriginX_ = 0;
	int pendingOriginY_ = 0;

	// ============================================================
	//  UI 状態
	// ============================================================

	EditorTool currentTool_ = EditorTool::NodePlace;

	// 無効なノード座標のセンチネル値
	static constexpr int INVALID = INT_MIN;

	/// 選択中ノードの座標。未選択時は {INVALID, INVALID}
	std::pair<int, int> selectedNode_ = { INVALID, INVALID };

	/// Connect / Disconnect ツールで 1 回目にクリックしたノード
	std::pair<int, int> connectFirstNode_ = { INVALID, INVALID };

	/// EntityPlace ツールで配置するエンティティ種別
	std::string entityTypeToPlace_ = "player";

	// ============================================================
	//  Save / Load 状態
	// ============================================================

	int  saveStageNumber_ = 1;
	int  loadStageNumber_ = 1;
	bool showOverwritePopup_ = false;

	/// 左ペインに表示するステータス文字列（成功=緑、エラー=赤）
	std::string statusMessage_;
	bool        statusIsError_ = false;

	// ============================================================
	//  ECS 再構築フラグ
	// ============================================================

	/// nodes_ が変更されたとき true にする。
	/// Update() の末尾で確認し、true なら RebuildGridEntities() を呼ぶ。
	bool dirty_ = false;

	// ============================================================
	//  定数
	// ============================================================

	/// ビューポート内の 1 セルのピクセルサイズ
	static constexpr float CELL_SIZE = 40.0f;

	// ============================================================
	//  ImGui 描画
	// ============================================================

	/// <summary>左ペイン：グリッド設定・ツール選択・ロード・セーブ</summary>
	void DrawLeftPane();

	/// <summary>中央ペイン：グリッドビューポート（DrawList で描画）</summary>
	void DrawCenterPane();

	/// <summary>右ペイン：選択ノードのプロパティ（接続フラグ・エンティティ）</summary>
	void DrawRightPane();

	/// <summary>上書き確認ポップアップ</summary>
	void DrawOverwritePopup();

	// ============================================================
	//  座標変換
	// ============================================================

	/// <summary>
	/// グリッド座標 → ImGui スクリーン座標（Y軸反転）
	/// </summary>
	ImVec2 GridToScreen(int gx, int gy, ImVec2 viewportOrigin) const;

	/// <summary>
	/// ImGui スクリーン座標 → グリッド座標（Y軸反転）
	/// グリッド範囲外の場合 false を返す。
	/// </summary>
	bool ScreenToGrid(ImVec2 screenPos, ImVec2 viewportOrigin, int& outX, int& outY) const;

	// ============================================================
	//  ノード・エンティティ操作
	// ============================================================

	/// ビューポートのクリックを currentTool_ に応じて振り分ける
	void HandleViewportClick(int gx, int gy);

	void PlaceNode(int gx, int gy);
	void DeleteNode(int gx, int gy);

	/// Connect / Disconnect ツールの 2 クリック処理
	/// isConnect=true で接続追加、false で接続削除
	void HandleConnectClick(int gx, int gy, bool isConnect);

	void PlaceEntity(int gx, int gy);
	void DeleteEntity(int gx, int gy);

	// ============================================================
	//  接続操作ヘルパー
	// ============================================================

	/// 隣接する 2 ノード間に接続を追加（双方向）
	void AddConnection(int ax, int ay, int bx, int by);

	/// 隣接する 2 ノード間の接続を削除（双方向）
	void RemoveConnection(int ax, int ay, int bx, int by);

	/// 2 ノードが上下左右のどれかで隣接しているか判定
	bool AreAdjacent(int ax, int ay, int bx, int by) const;

	// ============================================================
	//  Save / Load / Validate
	// ============================================================

	void ExecuteSave();
	void ExecuteLoad();

	/// セーブ前のバリデーション。NG の場合 errorMsg にメッセージを入れて false を返す。
	bool ValidateStage(std::string& errorMsg) const;

	/// std::ifstream で存在チェック（C++17 不要）
	bool FileExists(const std::string& path) const;

	// ============================================================
	//  ECS ヘルパー
	// ============================================================

	/// ECS から GridCellComponent を持つ全エンティティを削除
	void ClearGridEntities(No::Registry& registry);

	// ============================================================
	//  ユーティリティ
	// ============================================================

	void SetStatus(const std::string& msg, bool isError);
};



#endif // USE_IMGUI
