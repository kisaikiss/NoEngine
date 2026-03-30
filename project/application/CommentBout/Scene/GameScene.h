#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Editor/EditorManager.h"

class GameScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	void CameraImGui();
	void ChangeSceneImGui();
	void ClearOverConfigImGui();
	void PlayerDeathConfigImGui();

	No::Entity activeCameraEntity_{};
	No::Entity debugCameraEntity_{};
	No::Entity railCameraEntity_{};

	EditorManager editorManager_{};
};
