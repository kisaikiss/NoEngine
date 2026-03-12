#include "TestApp.h"
#include "Scene/TestScene.h"
#include "Scene/TestScene2.h"
#include "Scene/TestScene3.h"

void TestApp::Startup(void) {
	RegisterScene("TestScene", []() { return std::make_unique<TestScene>();	});
	RegisterScene("TestScene2", []() { return std::make_unique<TestScene2>();	});
	RegisterScene("TestScene3", []() { return std::make_unique<TestScene3>();	});
	ChangeScene("TestScene3");
}
