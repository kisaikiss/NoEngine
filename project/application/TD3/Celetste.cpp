#include "Celetste.h"
#include "Scene/SampleScene.h"
#include "Scene/GameScene.h"


void Celetste::Startup(void) {
	RegisterScene("Sample", []() { return std::make_unique<SampleScene>(); });
	RegisterScene("Game",   []() { return std::make_unique<GameScene>(); });
	ChangeScene("Game");
}
