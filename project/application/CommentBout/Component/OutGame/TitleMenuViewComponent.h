#pragma once
#include "engine/NoEngine.h"
#include <array>

struct TitleMenuViewComponent {
	No::Entity backgroundEntity = No::nullEntity;
	No::Entity panelEntity = No::nullEntity;
	No::Entity logoEntity = No::nullEntity;
	std::array<No::Entity, 3> itemEntities{};
	No::Entity cursorEntity = No::nullEntity;
};
