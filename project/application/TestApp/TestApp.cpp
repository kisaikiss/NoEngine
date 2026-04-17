#include "TestApp.h"
#include "Scene/EngineTestScene.h"

void TestApp::Startup(void) {
	RegisterScene("EngineTestScene", []() { return std::make_unique<EngineTestScene>();	});
	ChangeScene("EngineTestScene");
}
