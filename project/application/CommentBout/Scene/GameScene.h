#pragma once
#include "engine/NoEngine.h"

class GameScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void CameraImGui();

	void SpawnGrass(const No::Vector3& position, const No::Vector3& size);

	No::Entity activeCameraEntity_{};
	int grassNameIndex_ = 0;
};

