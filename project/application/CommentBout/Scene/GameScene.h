#pragma once
#include "engine/NoEngine.h"
#include <array>

class GameScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void CameraImGui();

	void SpawnGrass(const No::Vector3& position, const No::Vector3& size);
	void CreatePauseMenuSprites(const NoEngine::TextureRef& whiteTexture);

	No::Entity activeCameraEntity_{};

	// Pause メニュー用エンティティ
	No::Entity pauseDimEntity_{};
	No::Entity pauseMenuBgEntity_{};
	No::Entity pausePanelLineEntity_{};
	No::Entity pauseTitleEntity_{};
	std::array<No::Entity, 4> pauseItemEntities_{};
	No::Entity pauseCursorEntity_{};

	int grassNameIndex_ = 0;
};