#include "stdafx.h"
#include "CommentBout.h"
#include "Scene/GameScene.h"
#include "Scene/TitleScene.h"


void CommentBout::Startup(void) {
	RegisterScene("TitleScene", []() { return std::make_unique<TitleScene>();	});
	RegisterScene("GameScene", []() { return std::make_unique<GameScene>();	});
	ChangeScene("GameScene");
}
