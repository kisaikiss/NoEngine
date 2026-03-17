#include "stdafx.h"
#include "CommentBout.h"
#include "Scene/GameScene.h"


void CommentBout::Startup(void) {
	RegisterScene("GameScene", []() { return std::make_unique<GameScene>();	});
	ChangeScene("GameScene");
}
