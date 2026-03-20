#pragma once
#include "engine/NoEngine.h"

struct OptionStateComponent {
	enum OptionPhase {
		Closed = 0,
		Opening,
		OpenSelect,
		OpenEdit,
		Closing,
	};

	enum RequestedAction {
		None = 0,
		StartEdit,
		CloseOption,
	};

	bool isOpen = false;
	int phase = Closed;
	float phaseTime = 0.0f;
	float phaseDuration = 0.0f;

	int selectedIndex = 0;
	int itemCount = 5;
	bool isEditing = false;

	bool isConfirmAnimating = false;
	int confirmIndex = -1;
	float confirmAnimTime = 0.0f;
	int requestedAction = None;

	float masterVolume = 0.5f;
	float bgmVolume = 0.5f;
	float seVolume = 0.5f;
	bool vibrationEnabled = true;
};
