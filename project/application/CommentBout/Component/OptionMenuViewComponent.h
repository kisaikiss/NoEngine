#pragma once
#include "engine/NoEngine.h"
#include <array>

struct OptionMenuViewComponent {
	No::Entity dimEntity = No::nullEntity;
	No::Entity bgEntity = No::nullEntity;
	No::Entity lineEntity = No::nullEntity;
	No::Entity titleEntity = No::nullEntity;
	std::array<No::Entity, 5> itemEntities{};
	std::array<No::Entity, 5> labelEntities{};
	std::array<No::Entity, 3> barBaseEntities{};
	std::array<No::Entity, 3> barFillEntities{};
	No::Entity toggleEntity = No::nullEntity;
	No::Entity toggleOnEntity = No::nullEntity;
	No::Entity toggleOffEntity = No::nullEntity;
	No::Entity cursorEntity = No::nullEntity;
};
