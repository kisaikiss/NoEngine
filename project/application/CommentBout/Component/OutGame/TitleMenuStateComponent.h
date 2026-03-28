#pragma once

struct TitleMenuStateComponent {
	enum RequestedAction {
		None = 0,
		StartGame,
		OpenOption,
		ExitApp,
	};

	int selectedIndex = 0;
	int itemCount = 3;

	bool isConfirmAnimating = false;
	int confirmIndex = -1;
	float confirmAnimTime = 0.0f;
	int requestedAction = None;

	float logoMotionTime = 0.0f;
	bool isSceneChangePending = false;  // シーン遷移待機中 (フェード完了まで入力を遮断)
};
