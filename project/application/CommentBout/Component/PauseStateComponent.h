#pragma once
#include "engine/NoEngine.h"

struct PauseStateComponent {
	bool isPaused = false;
	bool justEnteredPause = false;
	bool justExitedPause = false;
	int selectedIndex = 0;
	int itemCount = 4;
};
