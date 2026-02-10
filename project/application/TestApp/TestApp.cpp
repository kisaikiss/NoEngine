#include "TestApp.h"
#include "Scene/TestScene.h"

void TestApp::Startup(void) {
	RegisterScene("TestScene", []() {
		return std::make_unique<TestScene>();
		});
	ChangeScene("TestScene");
}
