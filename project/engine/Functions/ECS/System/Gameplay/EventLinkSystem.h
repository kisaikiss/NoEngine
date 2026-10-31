#pragma once
#include "../ISystem.h"
#include "engine/Functions/ECS/Gameplay/GimmickGraph.h"

namespace NoEngine::ECS {
class EventLinkSystem : public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;

private:
	struct GraphExecution {
		Gameplay::GimmickGraph graph;
		Event::GameplayEvent event;
		std::vector<std::string> pendingNodes;
		std::string waitingNodeId;
		std::string waitingActionId;
		Entity waitingTarget = INVALID_ENTITY;
		nlohmann::json waitingProperties = nlohmann::json::object();
		size_t executedActions = 0;
		bool waiting = false;
	};

	std::vector<GraphExecution> graphExecutions_;
	void AdvanceGraphExecution(Registry& registry, GraphExecution& execution);
};
}
