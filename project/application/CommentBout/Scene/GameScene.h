#pragma once
#include "engine/NoEngine.h"

class GameScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void CameraImGui();

	void SpawnGrass(const No::Vector3& position, const No::Vector3& size);
	void UpdatePauseState();
	void UpdatePauseDim();
	void DrawPauseMenu();

	No::Entity activeCameraEntity_{};
	No::Entity pauseDimEntity_{};
	int grassNameIndex_ = 0;
};

