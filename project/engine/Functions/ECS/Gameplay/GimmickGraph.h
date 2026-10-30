#pragma once
#include "GameplayCatalog.h"

namespace NoEngine::Gameplay {

inline constexpr uint32_t kGimmickGraphVersion = 1;

namespace PinId {
inline constexpr char ExecInput[] = "exec.in";
inline constexpr char ExecOutput[] = "exec.out";
}

enum class GimmickNodeKind {
	Event,
	Action
};

// idは保存データ上の安定ID。imgui-node-editorのNodeIdとは独立させる。
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
	std::string id;
	std::string fromNodeId;
	std::string fromPinId;
	std::string toNodeId;
	std::string toPinId;
};

struct GimmickGraph {
	uint32_t version = kGimmickGraphVersion;
	std::vector<GimmickNode> nodes;
	std::vector<GimmickLink> links;

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

// エディタの接続操作・保存前検証・ランタイムのロード検証で共用する。
GimmickGraphValidation ValidateGimmickGraph(const GimmickGraph& graph,
	const GameplayCatalog& catalog = GameplayCatalog::Get());

}
