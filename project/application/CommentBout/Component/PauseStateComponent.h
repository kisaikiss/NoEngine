#pragma once
#include "engine/NoEngine.h"

struct PauseStateComponent {
	enum PausePhase {
		Closed = 0,
		Opening,
		Open,
		Closing,
		OptionOpening,
		OptionOpen,
		OptionClosing,
	};

	enum RequestedAction {
		None = 0,
		Resume,
		Restart,
		OpenOption,
		BackToTitle,
	};

	bool isPaused = false;
	bool justEnteredPause = false;
	bool justExitedPause = false;
	int selectedIndex = 0;
	int itemCount = 4;

	int phase = Closed;
	float phaseTime = 0.0f;
	float phaseDuration = 0.0f;
	int requestedAction = None;

	bool isConfirmAnimating = false;
	int confirmIndex = -1;
	float confirmAnimTime = 0.0f;
};
