#include "ClockworksDisease.h"
#include "Scene/TestScene.h"

void ClockworksDisease::Startup(void) {
	RegisterScene("TestScene", []() { return std::make_unique<TestScene>();	});
	ChangeScene("TestScene");
}
