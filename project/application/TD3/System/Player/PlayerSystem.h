#pragma once
#include "engine/NoEngine.h"
#include <string>

namespace Stage { class MapManager; }

class PlayerSystem : public No::ISystem {
public:
	void Initialize(No::Entity managerEntity, No::Entity playerEntity,
		No::Entity cam2dEntity, Stage::MapManager* mapManager);
	void Update(No::Registry& registry, float dt) override;

private:
	No::Entity managerEntity_ = 0;
	No::Entity playerEntity_ = 0;
	No::Entity cam2dEntity_ = 0;
	Stage::MapManager* mapMgr_ = nullptr;
	int lockedTransitionGx_ = -1;
	int lockedTransitionGy_ = -1;

	int lastStageNo_ = -1;  // ステージ切り替え検知用

	void Move(No::Registry& registry, float dt);
	void UpdateCamera(No::Registry& registry);
	void CheckTransition(No::Registry& registry);
	void CheckRespawnPoint(No::Registry& registry);
	void SyncRespawnToInitialSpawn(No::Registry& registry);
};
