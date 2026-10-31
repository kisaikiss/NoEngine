#pragma once
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/Gameplay/GimmickGraph.h"

namespace NoEngine::Editor {

class GimmickGraphPanel {
public:
	void Draw(ECS::Registry& registry, ECS::Entity selectedEntity);
	bool OwnsKeyboardFocus() const { return ownsKeyboardFocus_; }

private:
	std::string activePath_;
	Gameplay::GimmickGraph graph_;
	bool loaded_ = false;
	bool dirty_ = false;
	bool ownsKeyboardFocus_ = false;
	std::string status_;
	std::string contextNodeId_;
	std::string contextLinkId_;
	std::unordered_set<std::string> positionedNodeIds_;

	bool Load(const std::string& path);
	void Save();
};

}
