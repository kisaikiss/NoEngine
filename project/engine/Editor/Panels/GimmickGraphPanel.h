#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/Gameplay/GimmickGraph.h"

namespace NoEngine::Editor {

// UI表示中のGraphデータ。共有Documentにすることでパネル切替後もUndoコマンドが安全に参照できる。
struct GimmickGraphEditorDocument {
	Gameplay::GimmickGraph graph;
	bool dirty = false;
	uint64_t revision = 0;
};

class GimmickGraphPanel {
public:
	// 選択EntityのGimmickGraphComponentを読み、imgui-node-editor UIを描画・更新する。
	void Draw(ECS::Registry& registry, ECS::Entity selectedEntity);
	bool OwnsKeyboardFocus() const { return ownsKeyboardFocus_; }

private:
	std::string activePath_;
	std::shared_ptr<GimmickGraphEditorDocument> document_;
	bool loaded_ = false;
	bool ownsKeyboardFocus_ = false;
	std::string status_;
	std::string contextNodeId_;
	std::string contextLinkId_;
	std::unordered_set<std::string> positionedNodeIds_;
	std::optional<Gameplay::GimmickGraph> pendingMouseEditStart_;
	uint64_t observedDocumentRevision_ = 0;

	bool Load(const std::string& path);
	void Save();
};

}
