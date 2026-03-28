#include "TestApp.h"
#include "Scene/TestScene.h"
#include "Scene/TestScene2.h"

void TestApp::Startup(void) {
	RegisterScene("TestScene", []() { return std::make_unique<TestScene>();	});
	RegisterScene("TestScene2", []() { return std::make_unique<TestScene2>();	});
	ChangeScene("TestScene");
}
