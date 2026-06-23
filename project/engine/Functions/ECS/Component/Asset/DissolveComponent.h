#pragma once
namespace NoEngine {
namespace Component {
struct DissolveComponent {
	float threshold = 0.5f;
	std::string maskTextureName = "noise0";
};
}
}