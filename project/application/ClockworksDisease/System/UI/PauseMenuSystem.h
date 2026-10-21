#pragma once
#include "engine/NoEngine.h"

class PauseMenuSystem : public No::ISystem {
public:
	PauseMenuSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;

	static constexpr int kMainItemCount = 5;
	static constexpr int kControlLineCount = 6;
private:

	bool initialized_ = false;
	No::Entity menuEntity_ = No::INVALID_ENTITY;

	No::Entity backgroundEntity_ = No::INVALID_ENTITY;
	No::Entity mainTitleEntity_ = No::INVALID_ENTITY;
	No::Entity mainItemEntities_[kMainItemCount] = {};

	No::Entity optionTitleEntity_ = No::INVALID_ENTITY;
	No::Entity optionLabelEntity_ = No::INVALID_ENTITY;
	No::Entity optionValueEntity_ = No::INVALID_ENTITY;
	No::Entity optionHintEntity_ = No::INVALID_ENTITY;
	No::Entity controlsTitleEntity_ = No::INVALID_ENTITY;
	No::Entity controlLineEntities_[kControlLineCount] = {};
	No::Entity controlsHintEntity_ = No::INVALID_ENTITY;

	void CreateMenuEntities(No::Registry& registry);
	void OpenMenu(No::Registry& registry);
	void CloseMenu(No::Registry& registry);
	void UpdateMainMenu(No::Registry& registry);
	void UpdateOptionMenu(No::Registry& registry);
	void UpdateControlsMenu(No::Registry& registry);
	void RefreshMainMenuVisuals(No::Registry& registry);
	void RefreshOptionMenuVisuals(No::Registry& registry);
	void RefreshControlsMenuVisuals(No::Registry& registry);
	void SetPause(No::Registry& registry, bool isPause);
};
