#pragma once
#include "engine/NoEngine.h"
#include <array>

struct PauseMenuViewComponent {
	No::Entity dimEntity = No::nullEntity;
	No::Entity menuBgEntity = No::nullEntity;
	No::Entity panelLineEntity = No::nullEntity;
	No::Entity titleEntity = No::nullEntity;
	std::array<No::Entity, 4> itemEntities{};
	No::Entity cursorEntity = No::nullEntity;
};
