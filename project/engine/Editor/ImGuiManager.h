#pragma once

#include "engine/Runtime/Command/GraphicsContext.h"

namespace NoEngine {

class ImGuiManager {
public:
	void Initialize();
	void BeginFrame();
	void Render(GraphicsContext& context);
	void Shutdown();
};
}


