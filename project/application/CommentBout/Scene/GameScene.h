#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Editor/RailCameraEditor.h"
#include "application/CommentBout/Editor/GameEventEditor.h"
#include <array>

class GameScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void CameraImGui();
	void RailCameraImGui();
	void RailEditorImGui();
	void ChangeSceneImGui();
	void SpawnGrass(const No::Vector3& position, const No::Vector3& size);

	No::Entity activeCameraEntity_{};
	No::Entity debugCameraEntity_{};
	No::Entity railCameraEntity_{};

	RailCameraEditor railCameraEditor_{};
	GameEventEditor gameEventEditor_{};

	int grassNameIndex_ = 0;
};