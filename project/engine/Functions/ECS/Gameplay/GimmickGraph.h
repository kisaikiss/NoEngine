#pragma once
#include "GameplayCatalog.h"

namespace NoEngine::Gameplay {

// JSON保存形式の世代。形式を変更するときは読み込み互換処理と合わせて更新する。
inline constexpr uint32_t kGimmickGraphVersion = 1;

namespace PinId {
inline constexpr char ExecInput[] = "exec.in";
inline constexpr char ExecOutput[] = "exec.out";
}

enum class GimmickNodeKind {
	Event,
	Action
};

// グラフのノード本体。idは保存データ上の安定IDで、UIライブラリのNodeIdとは独立。
struct GimmickNode {
	std::string id;
	GimmickNodeKind kind = GimmickNodeKind::Action;
	// EventノードならEvent ID、ActionノードならAction ID。
	std::string typeId;
	float x = 0.0f;
	float y = 0.0f;
	nlohmann::json properties = nlohmann::json::object();
};

struct GimmickLink {
	std::string id;          // 保存データ上のリンクID
	std::string fromNodeId;  // 出力元Nodeの安定ID
	std::string fromPinId;   // 通常はexec.out
	std::string toNodeId;    // 入力先Nodeの安定ID
	std::string toPinId;     // 通常はexec.in
};

struct GimmickGraph {
	uint32_t version = kGimmickGraphVersion;
	std::vector<GimmickNode> nodes; // イベント開始点とAction処理
	std::vector<GimmickLink> links; // 実行順を定める有向接続

	nlohmann::json ToJson() const;
	static std::optional<GimmickGraph> FromJson(const nlohmann::json& json, std::string* error = nullptr);
};

bool SaveGimmickGraph(const std::filesystem::path& path, const GimmickGraph& graph,
	std::string* error = nullptr);
std::optional<GimmickGraph> LoadGimmickGraph(const std::filesystem::path& path,
	std::string* error = nullptr);

enum class DiagnosticSeverity {
	Warning,
	Error
};

struct GimmickGraphDiagnostic {
	DiagnosticSeverity severity = DiagnosticSeverity::Error;
	std::string code;
	std::string message;
	std::string nodeId;
	std::string linkId;
};

struct GimmickGraphValidation {
	std::vector<GimmickGraphDiagnostic> diagnostics;
	bool HasErrors() const;
};

// エディタで接続を受け付けるとき、保存前、ランタイムで読むときに共用する検証処理。
GimmickGraphValidation ValidateGimmickGraph(const GimmickGraph& graph,
	const GameplayCatalog& catalog = GameplayCatalog::Get());

}
