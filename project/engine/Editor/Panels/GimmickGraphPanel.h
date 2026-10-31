#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/Gameplay/GimmickGraph.h"

namespace NoEngine::Editor {

struct GimmickGraphEditorDocument {
	Gameplay::GimmickGraph graph;
	bool dirty = false;
	uint64_t revision = 0;
};

class GimmickGraphPanel {
public:
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
