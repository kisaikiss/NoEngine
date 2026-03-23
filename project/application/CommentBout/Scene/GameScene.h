#pragma once
#include "engine/NoEngine.h"
#include <array>

class GameScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void CameraImGui();
	void RailCameraImGui();
	void RailEditorImGui();

	void SpawnGrass(const No::Vector3& position, const No::Vector3& size);
	void CreatePauseMenuSprites(const NoEngine::TextureRef& whiteTexture);

	No::Entity activeCameraEntity_{};
	No::Entity debugCameraEntity_{};
	No::Entity railCameraEntity_{};

	int grassNameIndex_ = 0;
};