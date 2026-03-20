#pragma once
#include "engine/NoEngine.h"
#include <array>

class TitleScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void CreateOptionSprites(const NoEngine::TextureRef& whiteTexture);

	No::Entity optionDimEntity_ = No::nullEntity;
	No::Entity optionBgEntity_ = No::nullEntity;
	No::Entity optionLineEntity_ = No::nullEntity;
	No::Entity optionTitleEntity_ = No::nullEntity;
	std::array<No::Entity, 5> optionItemEntities_{};
	std::array<No::Entity, 5> optionLabelEntities_{};
	std::array<No::Entity, 3> optionBarBaseEntities_{};
	std::array<No::Entity, 3> optionBarFillEntities_{};
	No::Entity optionToggleEntity_ = No::nullEntity;
	No::Entity optionToggleOnEntity_ = No::nullEntity;
	No::Entity optionToggleOffEntity_ = No::nullEntity;
	No::Entity optionCursorEntity_ = No::nullEntity;
};