#include "TestApp.h"
#include "Scene/EngineTestScene.h"

void TestApp::Startup(void) {
	RegisterScene("TestScene", []() { return std::make_unique<EngineTestScene>();	});
	ChangeScene("TestScene");
}
