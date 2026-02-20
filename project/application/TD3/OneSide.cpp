#include "OneSide.h"
#include "Scene/SampleScene.h"


void OneSide::Startup(void) {
	RegisterScene("Sample", []() {
		return std::make_unique<SampleScene>();
		});
	ChangeScene("Sample");
}
