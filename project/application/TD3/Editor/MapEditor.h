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
///   配置 / 削除 の切り替えは左クリック / 右クリックで行う。
///     Node   : 左=ノード配置, 右=ノード削除
///     Connect: 左=接続(2クリック), 右=切断(2クリック)
///     Entity : 左=エンティティ配置, 右=エンティティ削除
/// </summary>
enum class EditorTool {
	Node,    // ノード操作
	Connect, // 接続 / 切断
	Entity   // エンティティ操作
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
	//  Undo / Redo スナップショット
	// ============================================================

	/// nodes_ と entities_ の丸ごとコピー。
	/// スナップショット方式を採用する理由:
	///   DrawCrossPanel / DrawPropertiesPanel の RadioButton が
	///   参照経由で nodes_ / entities_ を直接書き換えるため、
	///   コマンドパターンでは「変更前」の取得箇所がなくなる。
	struct EditorSnapshot {
		std::map<std::pair<int, int>, MapData::NodeData> nodes;
		std::vector<MapData::EntityData>                 entities;
	};

	std::vector<EditorSnapshot> undoStack_;  ///< Undo 履歴（末尾が最新）
	std::vector<EditorSnapshot> redoStack_;  ///< Redo 履歴（末尾が最新）

	static constexpr int MAX_UNDO = 50;      ///< 履歴の上限数

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

	int gridWidth_ = 10; // 編集可能範囲（幅）
	int gridHeight_ = 10; // 編集可能範囲（高さ）
	int viewOriginX_ = 0;  // ビューポート左端のグリッド X 座標
	int viewOriginY_ = 0;  // ビューポート下端のグリッド Y 座標（ゲーム座標）

	// Apply ボタンを押すまで実際の値に反映しない pending 値
	int pendingWidth_ = 10;
	int pendingHeight_ = 10;
	int pendingOriginX_ = 0;
	int pendingOriginY_ = 0;

	// ============================================================
	//  UI 状態
	// ============================================================

	EditorTool currentTool_ = EditorTool::Node;

	// 無効なノード座標のセンチネル値
	static constexpr int INVALID = INT_MIN;

	/// 選択中ノードの座標。未選択時は {INVALID, INVALID}
	std::pair<int, int> selectedNode_ = { INVALID, INVALID };

	/// Connect ツールで 1 回目にクリックしたノード
	std::pair<int, int> connectFirstNode_ = { INVALID, INVALID };

	/// Connect ツールの現在操作が「切断」かどうか
	/// false=接続（左クリック起動）/ true=切断（右クリック起動）
	bool connectIsDisconnecting_ = false;

	/// Entity ツールで配置するエンティティ種別
	std::string entityTypeToPlace_ = "player";

	// ============================================================
	//  Save / Load 状態
	// ============================================================

	int  saveStageNumber_ = 1;
	int  loadStageNumber_ = 1;
	bool showOverwritePopup_ = false;

	/// ステータス文字列（成功=緑、エラー=赤）
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

	/// "ツール・設定" ウィンドウ内容
	void DrawToolPanel();

	/// "マップビューポート" ウィンドウ内容
	void DrawViewportWindow();

	/// "プロパティ" ウィンドウ内容
	void DrawPropertiesPanel();

	/// 上書き確認ポップアップ（DrawToolPanel 内で呼ぶ）
	void DrawOverwritePopup();

	// ============================================================
	//  座標変換
	// ============================================================

	ImVec2 GridToScreen(int gx, int gy, ImVec2 viewportOrigin) const;
	bool ScreenToGrid(ImVec2 screenPos, ImVec2 viewportOrigin, int& outX, int& outY) const;

	// ============================================================
	//  ノード・エンティティ操作
	// ============================================================

	/// isRightClick=false→配置/接続, true→削除/切断
	void HandleViewportClick(int gx, int gy, bool isRightClick);

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

	///十字パネルを作成する関数
	void DrawCrossPanel(MapData::NodeData& node);

	// ============================================================
	//  Save / Load / Validate
	// ============================================================

	void ExecuteSave();
	void ExecuteLoad();
	bool ValidateStage(std::string& errorMsg) const;
	bool FileExists(const std::string& path) const;

	// ============================================================
	//  ECS ヘルパー
	// ============================================================

	void ClearGridEntities(No::Registry& registry);

	// ============================================================
	//  ユーティリティ
	// ============================================================

	void SetStatus(const std::string& msg, bool isError);

	// ============================================================
	//  Undo / Redo
	// ============================================================

	/// 現在の nodes_ / entities_ を Undo スタックに積む。
	/// Redo スタックはクリアされる。
	/// 必ず変更を加える直前に呼ぶこと。
	void PushUndo();

	/// 1つ前の状態に戻す。
	void Undo();

	/// 1つ先の状態へ進む。
	void Redo();
};

#endif // USE_IMGUI