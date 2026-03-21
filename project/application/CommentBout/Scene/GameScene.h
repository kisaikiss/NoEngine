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
	void CreateOptionSprites(const NoEngine::TextureRef& whiteTexture);

	No::Entity activeCameraEntity_{};
	No::Entity pauseDimEntity_{};
	No::Entity pauseMenuBgEntity_{};
	No::Entity pausePanelLineEntity_{};
	No::Entity pauseTitleEntity_{};
	std::array<No::Entity, 4> pauseItemEntities_{};
	No::Entity pauseCursorEntity_{};

	No::Entity optionDimEntity_{};
	No::Entity optionBgEntity_{};
	No::Entity optionLineEntity_{};
	No::Entity optionTitleEntity_{};
	std::array<No::Entity, 5> optionItemEntities_{};
	std::array<No::Entity, 5> optionLabelEntities_{};
	std::array<No::Entity, 3> optionBarBaseEntities_{};
	std::array<No::Entity, 3> optionBarFillEntities_{};
	No::Entity optionToggleEntity_{};
	No::Entity optionToggleOnEntity_{};
	No::Entity optionToggleOffEntity_{};
	No::Entity optionCursorEntity_{};
	int grassNameIndex_ = 0;
};

